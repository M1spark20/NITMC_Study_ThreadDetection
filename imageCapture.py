import io
import time
import picamera
from gpiozero import LED, Button
from datetime import datetime
from subprocess import check_call
import multiprocessing
import threading
import numpy as np
import sys

g_trigger = False
g_locker = multiprocessing.Lock()
g_lagDetector = 0
g_endFlag = False

class PhotoReflector(multiprocessing.Process):
	def __init__(self, trig, fps, isSmallSize, waitTime, printFlag, isDebug):
		multiprocessing.Process.__init__(self)
		self.led3 = LED(14)
		self.led4 = LED(11)
		self.led5 = LED(9)
		self.det  = Button(10)
		self.inputEnable = True
		self.prCheckNum = 0
		self.beginTime = time.time()
		self.led4.on()
		self.led5.off()

		self.trig = trig
		self.fps = fps
		self.waitTime = waitTime
		self.printFlag = printFlag
	# func __init__ end

	def run(self):
		slp = self.waitTime - 0.5/self.fps
		while True:
			time.sleep(0.0005)
			if not self.trig.value < 0:
				continue
			if self.inputEnable and self.det.is_pressed:
				self.led4.off()
				self.led5.on()
				time.sleep(slp)
				self.trig.value = time.time()-slp
				self.inputEnable = False
				time.sleep(0.3)
				self.led5.off()
			if not self.inputEnable and not self.det.is_pressed:
				self.inputEnable = True
				self.led4.on()
			# end if

			# calculate tps every second
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

class DetectCamera(threading.Thread):
	def __init__(self, trig, fps, isSmallSize, waitTime, printFlag, isDebug):
		threading.Thread.__init__(self)
		self.trig = trig
		self.camera = picamera.PiCamera()
		if isSmallSize:
			self.camera.resolution = (320, 240)
		else:
			self.camera.resolution = (640, 480)
		self.camera.exposure_mode = 'off'
		self.camera.awb_mode = 'off'
		self.camera.shutter_speed = 666 #[us]
		self.camera.awb_gains = (1.5, 1.5)
		self.camera.iso = 1600
		self.camera.framerate = fps
		self.camera.start_preview()
		time.sleep(5)		# camera setup
		self.camera.stop_preview()
		self.stream = io.BytesIO()
		self.beginTime = time.time()
		self.captureImages = 0
		self.captureNo = 0

		self.waitTime = waitTime
		self.printFlag = printFlag
		self.detectParam = ['./threadDetect', 'param.cfg']
		if isDebug:
			self.detectParam.append('-d')
		if isSmallSize:
			self.detectParam.append('-s')
	# func __init__ end

	def run(self):
		global g_endFlag
		capTime_list = [] #[ms]
		capMin = -1
		capMax = -1
		overTime = 0
		analyzeFlag = False
		capFlag = False
		capTime = time.time()
		start_time = time.time()
		end_time = time.time()
		for foo in self.camera.capture_continuous(self.stream, format='bgr', use_video_port=True):
			capTime = time.time() - capTime
			#print('snapped: %.1f[ms]' % (1000*capTime))
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
			if not self.trig.value < 0:
				print('catch jitter: %.2f[ms]' % (1000*(time.time()-self.trig.value-self.waitTime)))
				print('delay: %.2f[ms], capTime:%.2f[ms]' % (self.waitTime*1000, capTime*1000))
				name = datetime.now().strftime('%s') + '-' + str(self.captureNo) + '.bgr'
				with open(name, 'wb') as writer:
					writtenBytes = writer.write(self.stream.getvalue())
				# with writer end
				param = self.detectParam[:]
				param.append(name)
				check_call(param)

				check_call(["rm", name])
				self.trig.value = -1.0
				self.captureNo += 1
				capFlag = False
			#end if
			self.stream.seek(0)		# set stream current to front
			self.stream.truncate()	# fix stream size to 0 (=reset)

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

			capTime = time.time()
			end_time = capTime 
		# end for
	# def run end
# class Camera end definition

# main function
if __name__ == '__main__':
	# name, fps, isSmallSize, waitTime[ms], printFlag, debugFlag
	try:
		argv = sys.argv
		if not len(argv) == 6:
			print('!! set argv as fps, isSmallSize[T/F], waitTime[ms], printFlag[T/F], debugFlag[T/F]')
			exit()
		smallFlag = True
		printFlag = True
		debugFlag = True
		l_fps = float(argv[1])
		l_waitTime = float(argv[3])/1000
		if not 'T' in argv[2]:
			smallFlag = False
		if not 'T' in argv[4]:
			printFlag = False
		if not 'T' in argv[5]:
			debugFlag = False
		triggerCarrier = multiprocessing.Value('d',-1.0)
		cam = DetectCamera(trig=triggerCarrier, fps=l_fps, isSmallSize=smallFlag, waitTime=l_waitTime, printFlag=printFlag, isDebug=debugFlag)
		pr  = PhotoReflector(trig=triggerCarrier, fps=l_fps, isSmallSize=smallFlag, waitTime=l_waitTime, printFlag=printFlag, isDebug=debugFlag)
		cam.start()
		pr.start()

		while cam.is_alive() or pr.is_alive():
			pr.join(0.2)
			cam.join(0.2)			

	except KeyboardInterrupt:
		if printFlag:
			print('')
			print('!! Ctrl-C detected: Program will be aborted.')
		pr.terminate()
		g_endFlag = True
# end main
