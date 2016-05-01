# Team Id: 10
# Author List: Vishal Agarwal, Pratham Desai, Darsh Shah and Amal Dani
# Filename: is_talking.py
# Functions: main()
# Global Variables: NONE

from pyAudioAnalysis import audioTrainTest as aT
import subprocess
import time

duration = 5
tapx = "50"
tapy = "700"
fileName = "input.wav"
isReduced = 0
deltaSound = 5
speechThreshold = 0.5

subprocess.call("sudo adb shell rm /sdcard/Recorders/*.mp3", shell=True)

for i in xrange(1,100):
	# Start recording
	subprocess.call(["sudo", "adb", "shell", "input", "tap", tapx, tapy])
	time.sleep(duration)
	# Stop recording
	subprocess.call(["sudo", "adb", "shell", "input", "tap", tapx, tapy])

	# Transfer file from phone to rpi
	subprocess.call(["sudo", "adb", "pull", "/sdcard/Recorders/", "./"])
	subprocess.call(["sudo", "adb", "shell", "rm", "-r", "/sdcard/Recorders/*"])

	# Convert file from mp3 to wav
	subprocess.call("mv *.mp3 input.mp3", shell=True)
	subprocess.call(["avconv", "-loglevel", "panic", "-i", "input.mp3", fileName])
	subprocess.call(["rm", "-f", "input.mp3"])

	# Classify
	[Result, P, classNames] = aT.fileClassification(fileName, "./pyAudioAnalysis/data/svmSM","svm")
	print ("-----------------------------------")
	print Result, P, classNames

	if P[0] > speechThreshold and isReduced == 0:
		for i in xrange(0,deltaSound):
			subprocess.call(["sudo", "adb", "shell", "input", "keyevent", "25"])
		isReduced = 1

	if P[0] < speechThreshold and isReduced == 1:
		for i in xrange(0,deltaSound):
			subprocess.call(["sudo", "adb", "shell", "input", "keyevent", "24"])
		isReduced = 0

	print ("-----------------------------------")

	subprocess.call(["rm", "-f", fileName])
