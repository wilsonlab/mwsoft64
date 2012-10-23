#!/usr/bin/python

import os 
import shutil
from subprocess import call 


##############################
# Define data files
##############################
eeg_files = { 'raw' 	:  '../data/original.eeg.raw', 
			  'ad-ext'	: '../data/original.eeg'}

pos_files = { 'raw'		: '../data/original.pos.raw',
			  'ad-ext'	: '../data/original.pos',
			  'pos-ext'	: '../data/original.p'}

spk_files = { 'raw'		: '../data/original.tt,raw',
			  'ad-ext' 	: '../data/original.tt'}

tt_files  = {'tt'	: 'original2.tt',
			 'parm'	: 'original2.pxyabw',
			 'cb'	: 'original2.cb',
			 'ave'	: 'original2.ave'}

tmpDir = '/tmp/mwltemp'

def mkTmpDir():
	if not os.path.isdir(tmpDir):
		os.makedirs(tmpDir)

def rmTmpDir():
	if os.path.isdir(tmpDir):
		shutil.rmtree(tmpDir)

def runTests():
	mkTmpDir()
	testExtractEEG()
	testExtractPos()
	testExtractTet()
#	rmTmpDir()


def testExtractEEG():
	print 'Testing: adextract for eeg'
	cmd = 'adextract -eslen80 ' + eeg_files['raw'] + ' -c -o ' + tmpDir + '/test.eeg'
	print cmd
	call( [cmd], shell=True )
	


def testExtractPos():
	print 'Testing: adextract for position'
	print 'Testing: posextract'
	
def testExtractTet():
	print 'Testing: adextract for tetrodes'

def testSpikeParms():
	print 'Testing: spikeparms'


def diffBinaryFiles(file1, file2):
	return True

if __name__ =='__main__':
	runTests()

