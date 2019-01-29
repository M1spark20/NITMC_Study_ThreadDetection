import io
import time
import picamera
from gpiozero import LED, Button
from datetime import datetime
from subprocess import call, check_call
import multiprocessing
import threading
import numpy as np
import sys

g_trigger = False
g_locker = multiprocessing.Lock()
g_lagDetector = 0
g_endFlag = False

# フォトリフレクタ制御関数
class PhotoReflector(multiprocessing.Process):
	# 初期化処理
	def __init__(self, trig, fps, isSmallSize, waitTime, printFlag, isDebug):
		multiprocessing.Process.__init__(self)
		self.led4 = LED(11)				# 検知状況インジケータ1
		self.led5 = LED(9)				# 検知状況インジケータ2
		self.det  = Button(10)			# フォトリフレクタ入力信号
		self.inputEnable = False		# 信号の立ち上がり判定フラグ
		self.prCheckNum = 0				# 1秒間の検知回数把握用変数
		self.beginTime = time.time()	# 1秒間の検知回数把握用変数
		self.led4.on()					# インジケータ初期設定
		self.led5.off()					# インジケータ初期設定

		# 外部変数入力
		self.trig = trig				# トリガ変数
		self.fps = fps					# カメラの撮影周期
		self.waitTime = waitTime		# シャトル検知後待機時間
		self.printFlag = printFlag		# ログ出力フラグ
	# func __init__ end

	def run(self):
		slp = self.waitTime - 0.5/self.fps		# 待機時間を指定: 待機時間から撮影周期の半分を減算→タイミングを+-fps/2とする
		if slp < 0:
			slp = 0
		while True:								# 終了まで検知を継続する
			time.sleep(0.0005)
			if not self.trig.value < 0:			# トリガがDetectCameraクラスにより解除されていない場合処理を行わない
				continue
			self.led5.off()						# トリガ発生インジケータ消灯

			# トリガ信号の送出時の処理
			if self.inputEnable and not self.det.is_pressed:
				self.led4.off()						# シャトル検知中インジケータ消灯
				time.sleep(slp)						# 一定時間待機して、横糸を撮影範囲に引き込む
				self.led5.on()						# トリガ発生インジケータ点灯
				self.trig.value = time.time()-slp	# トリガ発生(データはトリガ発生時間)
				self.inputEnable = False			# 信号発生enableオフ
				time.sleep(0.3)						# シャトルが通過するまで待機
			# end if

			# 検知信号であるフォトリフレクタ回路コンパレータ出力の立ち下がり(reset)を検知した場合
			if not self.inputEnable and self.det.is_pressed:
				self.led4.on()				# シャトル検知中インジケータ点灯
				self.inputEnable = True		# 信号発生enableオン
			# end if

			# 1秒間のフォトリフレクタ信号チェック回数の計算
			self.prCheckNum += 1
			nowTime = time.time()
			if nowTime - self.beginTime >= 1:
				if self.printFlag:
					print('photo reflector check times/sec.: %.2f[tps]' % (self.prCheckNum / (nowTime - self.beginTime)))
				self.prCheckNum = 0
				self.beginTime = nowTime
			# end if
		# end while
	# func run end
#class PhotoReflector end definition

# 画像撮影・プログラム起動
class DetectCamera(threading.Thread):
	# 初期化処理
	def __init__(self, trig, fps, isSmallSize, waitTime, printFlag, isDebug, ss, ISO):
		threading.Thread.__init__(self)

		# カメラの初期化
		self.camera = picamera.PiCamera()
		if isSmallSize:									# 撮影サイズ指定
			self.camera.resolution = (320, 240)			# 小さいサイズ
		else:
			self.camera.resolution = (640, 480)			# 大きいサイズ
		self.camera.exposure_mode = 'off'				# 露出の自動調整をOFFにする
		self.camera.awb_mode = 'off'					# ホワイトバランスの自動調整をOFFにする
		print(ss)
		self.camera.shutter_speed = ss #[us] = 1/1,500	# シャッター速度を指定
		self.camera.awb_gains = (1.5, 1.5)				# ホワイトバランスを指定
		self.camera.iso = ISO							# ISO感度を指定
		self.camera.framerate = fps						# 撮影速度を指定
		self.camera.start_preview()						# カメラ撮影状況確認・内部自動調整
		time.sleep(5)									# 調整に十分な時間待機
		self.camera.stop_preview()

		# バッファ・実測fps計測用変数定義
		self.stream = io.BytesIO()						# 画像データ一次格納先バッファ
		self.beginTime = time.time()					# 撮影開始時間規定
		self.captureImages = 0							# 撮影した枚数
		self.captureNo = 0								# 出力した画像の枚数: 画像ファイル名につける

		# パラメータ読み込み
		self.trig = trig								# トリガ変数
		self.waitTime = waitTime						# シャトル検知後待機時間
		self.printFlag = printFlag						# ログ出力フラグ
		
		# インジゲータ定義
		self.powerLED = LED(13)
		self.okStateLED = LED(19)
		self.ngStateLED = LED(26)
		
		# C++プログラム呼び出し用引数指定(パラメータファイル: param.cfg)
		self.detectParam = ['./threadDetect', 'param.cfg']	# 共通項
		if isDebug:											# デバッグオプション時は-dを付加
			self.detectParam.append('-d')
		if isSmallSize:										# 出力画像が320x240の時は-sを付加
			self.detectParam.append('-s')
	# func __init__ end

	def run(self):
		global g_endFlag			# グローバルパラメータとして終了フラグを規定
		capTime_list = [] 			# プログラムの画像取得間隔保存先[ms]: 統計計算に使用
		capMin = -1					# 取得間隔最小値
		capMax = -1					# 取得間隔最大値
		overTime = 0				# 特定の取得間隔=30[ms]を超えた枚数
		analyzeFlag = False			# 初回の取得間隔スキップ用
		capFlag = False				# 未使用
		capTime = time.time()		# 撮影間隔測定用変数
		start_time = time.time()	# 撮影開始時間
		end_time = time.time()		# 撮影終了時間
		last_catch = start_time		# 最終画像取得時間
		self.powerLED.on()		# プログラム動作LED点灯

		# キャプチャ: RTOSから50fpsで信号を取得→受け取り次第処理を行う
		for foo in self.camera.capture_continuous(self.stream, format='bgr', use_video_port=True):
			capTime = time.time() - capTime
			#print('snapped: %.1f[ms]' % (1000*capTime))
			# 終了判定: 終了時はログ出力を行う場合撮影状況に関する各種統計量を計算し出力
			if g_endFlag:
				if self.printFlag:
					print('taken images   : %d'   % (len(capTime_list)))
					print('setting f.Rate : %.2f[fps]' % (self.camera.framerate))
					print('whole frameRate: %.4f[fps]' % (len(capTime_list)/(end_time-start_time)))
					print('minimum  time  : %.2f[ms]' % (min(capTime_list)))
					print('maximum  time  : %.2f[ms]' % (max(capTime_list)))
					print('mean     time  : %.2f[ms]' % (np.mean(capTime_list)))
					print('median   time  : %.2f[ms]' % (np.median(capTime_list)))
					print('variance time  : %.2f[ms]' % (np.var(capTime_list)))
					print('stdev    time  : %.2f[ms]' % (np.std(capTime_list)))
					print('over 30[ms]    : %d / %.4f[%%]' % (overTime, overTime/len(capTime_list)*100))
				break
			# トリガ発生時処理: 撮影した画像をファイルに保存してC++プログラムを呼び出す
			if not self.trig.value < 0:
				self.okStateLED.off()
				self.ngStateLED.off()
				#print('catch jitter: %.2f[ms]' % (1000*(time.time()-self.trig.value-self.waitTime)))
				spotTime = time.time()-self.trig.value	# 実待機時間を計算
				last_catch = time.time()-last_catch		# 前回撮影からの撮影間隔を計算
				# 上記2つのデータを出力
				print('delay: %.2f[ms], spotTime:%.2f[ms], catch interval: %.2f[ms]' % (self.waitTime*1000, spotTime*1000, last_catch*1000))
				last_catch = time.time()				# 最終画像取得時間を更新
				
				# ファイル名を指定してbgr画像を書き出し: ファイル名は(Linux時間)-(累計出力枚数)
				name = datetime.now().strftime('%s') + '-' + str(self.captureNo) + '.bgr'
				with open(name, 'wb') as writer:
					writtenBytes = writer.write(self.stream.getvalue())
					
				# C++パラメータにファイル名を追加してC++ファイルを起動
				param = self.detectParam[:]
				param.append(name)
				cppRes = call(param)
				
				# リザルトを表示
				if cppRes == 0:		# OK
					self.okStateLED.on()
				elif cppRes == 1:	# NG
					self.ngStateLED.on()

				check_call(["rm", name])	# bgrファイル削除(-dオプション使用時はC++によりbmp形式のファイル生成済)
				self.trig.value = -1.0		# トリガをクリアする
				self.captureNo += 1			# 出力した画像の枚数を更新
				capFlag = False				# 未使用
			#end if
			
			# バッファのクリアを行う
			self.stream.seek(0)			# set stream current to front
			self.stream.truncate()		# fix stream size to 0 (=reset)

			# ignore first shot for analyzing(because of setting up)
			if analyzeFlag:
				if capMin == -1 or capMin > capTime:
					capMin = capTime
				if capMax == -1 or capMax < capTime:
					capMax = capTime
				if capTime > 0.03:
					overTime += 1
				capTime_list.append(capTime*1000)
			else:
				analyzeFlag = True

			# calculate fps every second
			self.captureImages += 1
			nowTime = time.time()
			if nowTime - self.beginTime >= 1:
				if self.printFlag:
					print('Now capturing: %.2f[fps] min: %.2f[ms], max: %.2f[ms]' % (self.captureImages / (nowTime - self.beginTime), capMin*1000, capMax*1000))
				self.captureImages = 0
				self.beginTime = nowTime
				capMin = -1
				capMax = -1
			# end if

			capTime = time.time()	# 撮影間隔測定用に時間を設定
			end_time = capTime 		# 撮影終了時間更新
		# end for
	# def run end
# class Camera end definition

# main function
if __name__ == '__main__':
	# params.: name, fps, waitTime[ms], ss[1/s], ISO, isSmallSize[T/F], printFlag[T/F], debugFlag[T/F]
	try:
		# コマンドライン引数を読み込み: 8個パラメータがあるかチェック
		argv = sys.argv
		if not len(argv) == 8:
			# なければ入力フォーマットを示し入力を促してプログラム終了
			print('!! set argv as fps, waitTime[ms], ss[1/s], ISO, isSmallSize[T/F], printFlag[T/F], debugFlag[T/F]')
			exit()
		smallFlag = True
		printFlag = True
		debugFlag = True
		l_fps = float(argv[1])				# 撮影速度
		l_waitTime = float(argv[2])/1000	# 待機時間
		l_ss = int(1000000/int(argv[3]))	# シャッター速度
		l_ISO = int(argv[4])				# ISO感度
		if not 'T' in argv[5]:				# bgr画像サイズ(T:320x240, F:640x480)
			smallFlag = False
		if not 'T' in argv[6]:				# ログ出力フラグ
			printFlag = False
		if not 'T' in argv[7]:				# デバッグオプションフラグ
			debugFlag = False
		triggerCarrier = multiprocessing.Value('d',-1.0)	# トリガ信号伝達用変数
		
		# DetectCameraクラスおよびPhotoReflectorクラスを初期化して起動
		cam = DetectCamera(trig=triggerCarrier, fps=l_fps, isSmallSize=smallFlag, waitTime=l_waitTime, printFlag=printFlag, isDebug=debugFlag, ss=l_ss, ISO=l_ISO)
		pr  = PhotoReflector(trig=triggerCarrier, fps=l_fps, isSmallSize=smallFlag, waitTime=l_waitTime, printFlag=printFlag, isDebug=debugFlag)
		cam.start()
		pr.start()
		
		# 0.2秒ごとに各クラスの状態を監視しながら処理を実行: 各クラスのrun関数が実施される
		while cam.is_alive() or pr.is_alive():
			pr.join(0.2)
			cam.join(0.2)

	except KeyboardInterrupt:	# Ctrl+C押下時にプログラムを終了: 暫定
		if printFlag:
			print('')
			print('!! Ctrl-C detected: Program will be aborted.')
		pr.terminate()
		g_endFlag = True
# end main
