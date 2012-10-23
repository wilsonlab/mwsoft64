#!/usr/bin/python

import os 
import shutil
from subprocess import call 
import filecmp
import sys


##############################
# Define data files
##############################
eegFiles = { 'raw' 	:  './data/original.eeg.raw', 
			  'ad-ext'	: './data/original.eeg'}

posFiles = { 'raw'		: './data/original.pos.raw',
			 'ad-ext'	: './data/original.pos',
			 'pos-ext'	: './data/original.p'}

spkFiles = { 'raw'		: './data/original.tt.raw',
			 'ad-ext' 	: './data/original.tt'}

ttFiles  = {'tt'	: './data/original2.tt',
			'parm'	: './data/original2.pxyabw',
			'cb'	: './data/original2.cb',
			'ave'	: './data/original2.ave'}

tmpDir = '/tmp/mwltemp/'

def runTests():
	mkTmpDir()
	testExtractEEG()
	testExtractPos()
	testExtractTet()
	testSpikeParms()
	testSpikeAve()
	#rmTmpDir()


def testExtractEEG():
	print 'ADEXTRACT eeg...', 
	
	outFile = tmpDir + 'test.eeg';
	cmd = './bin/adextract -eslen80 ' + eegFiles['raw'] + ' -c' + ' -o '
	executeCmd(cmd)
	testResult(eegFiles['ad-ext'], outFile, cmd)

def testExtractPos():
	print 'ADEXTRACT POS...',
	
	outFile = tmpDir + 'test.pos'
	cmd = './bin/adextract ' + posFiles['raw'] + ' -o ' + outFile
	executeCmd(cmd)
	testResult(posFiles['ad-ext'], outFile, cmd);

	print 'POSEXTRACT POS...',

	outFile = tmpDir + 'test.p';
	cmd = './bin/posextract ' + posFiles['ad-ext'] + ' -o ' + outFile
	executeCmd(cmd)
	testResult(posFiles['pos-ext'], outFile, cmd)

def testExtractTet():
	print 'ADEXTRACT TET...',

	outFile = tmpDir + 'test.tt'
	cmd = './bin/adextract -eslen80 ' + spkFiles['raw'] + ' -t -probe 0 -o ' + outFile
	executeCmd(cmd)
	testResult(spkFiles['ad-ext'], outFile, cmd)

def testSpikeParms():
	print 'SPIKEPARMS TET...',

	outFile = tmpDir + 'test.pxyabw'
	cmd = './bin/spikeparms2 ' + ttFiles['tt'] + ' -tetrode -parms t_px,t_py,t_pa,t_pb -o ' + outFile
	executeCmd(cmd)
	testResult( ttFiles['parm'], outFile, cmd)	

def testSpikeAve():
	print 'SPIKEAVGE TET..',

	outFile = tmpDir + 'test.ave'
	cmd = './bin/spikeavg ' + ttFiles['tt'] + ' -if ' + ttFiles['cb'] + ' > ' + outFile
	executeCmd(cmd)
	testResult(ttFiles['ave'], outFile, cmd)	

def executeCmd(cmd):
	with open('/dev/null', "w") as fnull:
		result = call(cmd, shell=True, stdout = fnull, stderr = fnull)

def testResult(orig, new, cmd):
	newFileSize = os.stat(new).st_size;

	if diffBinaryFiles(orig, new) and not newFileSize==0:
		print '\033[92m' + '\tPASSED!' +  '\033[0m'
	else:
		print '\033[91m' + '\tFAILED!' +  '\033[0m'

def diffBinaryFiles(orig, new):
	out = [tmpDir + 'out1', 	tmpDir + 'out2']
	
	cmd = [ "sed '1,/%%ENDHEADER/ d' " + orig + ' > ' + out[0], 
		    "sed '1,/%%ENDHEADER/ d' " + new + ' > ' + out[1] ]
	
	call([cmd[0]], shell=True)
	call([cmd[1]], shell=True)
	
	return filecmp.cmp(out[0], out[1])

def mkTmpDir():
	if not os.path.isdir(tmpDir):
		os.makedirs(tmpDir)

def rmTmpDir():
	if os.path.isdir(tmpDir):
		shutil.rmtree(tmpDir)	

if __name__ =='__main__':
	runTests()

