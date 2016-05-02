# Team Id: 10
# Author List: Vishal Agarwal, Pratham Desai, Darsh Shah and Amal Dani
# Filename: music_vs_speech.py
# Functions: main()
# Global Variables: NONE

from pyAudioAnalysis import audioTrainTest as aT
import subprocess
import time

for i in xrange(1,100):
	# Recording audio
	subprocess.call(["arecord", "-d", "5", "-D", "plughw:1", "output.wav"])

	# Classifying the recorded file
	outFileName = "./output.wav"
	[Result, P, classNames] = aT.fileClassification(outFileName, "./pyAudioAnalysis/data/svmSM","svm")
	print Result, P, classNames

	# Changing the channel if probability of speech is > 0.95
	if P[0] > 0.95:
		print("Changing the channel...\n")
		subprocess.call(["sudo", "adb", "shell", "input", "tap", "632", "978"])
		time.sleep(5)