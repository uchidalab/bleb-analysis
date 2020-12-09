# @File(label = "Input directory", style = "directory") srcFile
# @File(label = "Output directory", style = "directory") dstFile
# @String(label = "File extension", value=".png") given_ext
# @String(label = "Noise tolerance", value = "100") noiseNum

import os
from ij import IJ, ImagePlus
from ij.process import ImageProcessor
from ij.plugin.filter import MaximumFinder
from ij.measure import ResultsTable


def getDirList(super_path):
	dir_list = []
	for item in os.listdir(super_path):
		if os.path.isdir(super_path + item):
			dir_list.append(item)
	return sorted(dir_list)

def getFileList(dir_path, ext='png'):
	print(ext)
	file_list = []
	for item in os.listdir(dir_path):
		if os.path.isfile(dir_path + item):
			file_name, file_ext = os.path.splitext(item)
			if file_ext == ('.' + ext):
				file_list.append(item)
	return sorted(file_list)

def makeOutDir(path):
	if os.path.isdir(path):
		print('output dir is aleady exist.')
	else:
		os.makedirs(path)
		print('made output dir:')
		print(path)

def process(currentDir, dstDir, fileName, counter):
	# Opening the image
	IJ.log("Open image file:" + str(fileName))
	imp = IJ.openImage(os.path.join(currentDir, fileName))

	# findMaxima for output format [Point Selection]
	# IJ.run(imp, "Find Maxima...", "noise=100 output=[Point Selection]")
	# imp.show()

	# findMaxima for output format [List]
	IJ.run(imp, "Find Maxima...", "noise=" + noiseNum +  " output=List")
	IJ.saveAs("Results", dstDir + "/%05d.csv"%counter)

def main():
	# select folders
	srcDir = srcFile.getAbsolutePath() + '/'
	dstDir = dstFile.getAbsolutePath() + '/findmaxima/'
	print(srcDir)
	print(dstDir)
	cat_list = getDirList(srcDir)
	print(cat_list)
	for cat_name in cat_list:
		cat_path = srcDir + cat_name + '/'
		subdir_list = getDirList(cat_path)
		print(subdir_list)
		for dir_name in subdir_list:
			target_path = cat_path + dir_name + '/'
			output_path = dstDir + cat_name + '/' + dir_name
			makeOutDir(output_path)
			print('outdir')
			file_list = getFileList(target_path, ext=given_ext)
			for i, file_name in enumerate(file_list):
				print('Processing:' + target_path + file_name)
				# processing findMaxima
				process(target_path, output_path, file_name, i)
	IJ.log("finish!")

# Program start
main()
