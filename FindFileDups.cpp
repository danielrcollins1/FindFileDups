/*
	Name: FindFileDups
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 10-10-22 04:35
	Description: Find files in working directory
		with matching file sizes: prospects for duplicate files.
*/
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <Windows.h>
using namespace std;

// Typedefs for file data structures
typedef WIN32_FIND_DATA FileData;
typedef vector<FileData> FileList;
typedef size_t Index;

// Get all files in working directory
FileList getAllFiles() {
	FileData fd;
	FileList files;
	HANDLE hFind = FindFirstFile("*.*", &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				files.push_back(fd);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return files;
}

// Compare size of two files,
//   considering the two size fields per file.
// Returns -1 if first smaller, 0 if equal, +1 if first bigger
int compareFileSizes (const FileData &fd1, const FileData &fd2) {

	// Compare high fields
	if (fd1.nFileSizeHigh < fd2.nFileSizeHigh)
		return -1;
	else if (fd1.nFileSizeHigh > fd2.nFileSizeHigh)
		return +1;

	// Compare low fields (high fields equal)
	if (fd1.nFileSizeLow < fd2.nFileSizeLow)
		return -1;
	else if (fd1.nFileSizeLow > fd2.nFileSizeLow)
		return +1;

	// All fields are equal
	return 0;
}

// Compare two file contents
// Return 0 if the same, nonzero if different
int compareFileContents (const char* filename1, const char* filename2) {
	int retval = 0;
	fstream file1, file2;
	file1.open(filename1, ios::in | ios::binary);
	file2.open(filename2, ios::in | ios::binary);
	while (file1 && file2) {
		char c1, c2;
		file1.read(&c1, sizeof(c1));
		file2.read(&c2, sizeof(c2));
		if (c1 != c2) {
			retval = c1 < c2 ? -1 : +1;
			break;
		}
	}
	file1.close();
	file2.close();
	return retval;
}

// Compare two file contents from FileData
int compareFileContents (const FileData &fd1, const FileData &fd2) {
	return compareFileContents(fd1.cFileName, fd2.cFileName);
}

// Compare two files in toto
// First checks size, then contents
// Return 0 if the same, nonzero if different
int compareFiles (const FileData &fd1, const FileData &fd2) {
	return !(compareFileSizes(fd1, fd2) == 0
		&& compareFileContents(fd1, fd2) == 0);
}

// Selection sort the file list on total size
void selectionSortFileList(FileList &list) {
	FileData minValue;
	Index startScan, minIndex;
	for (startScan = 0; startScan < list.size() - 1; startScan++) {
		minIndex = startScan;
		minValue = list[startScan];
		for(Index index = startScan + 1; index < list.size(); index++) {
			if (compareFileSizes(list[index], minValue) < 0) {
				minValue = list[index];
				minIndex = index;
			}
		}
		list[minIndex] = list[startScan];
		list[startScan] = minValue;
	}
}

// Compute the total size of a file in KB
unsigned int fileSizeInKB (const FileData &fd) {
	const unsigned KB_IN_BYTES = 1024;
	const unsigned HIGH_KB_UNITS = MAXDWORD / KB_IN_BYTES;
	return fd.nFileSizeHigh * HIGH_KB_UNITS
		+ fd.nFileSizeLow / KB_IN_BYTES + 1;
}

// Report info on one file
void reportFile (const FileData &fd) {
	cout << fileSizeInKB(fd) << " " << fd.cFileName << endl;
}

// Scan sorted listed for identical files
void reportIdenticalFiles (FileList files) {
	bool inEqualBlock = false;
	cout << "Identical files report (size in KB):\n\n";
	for (Index i = 0; i < files.size() - 1; i++) {
		if (compareFiles(files[i], files[i+1]) == 0) {
			reportFile(files[i]);
			inEqualBlock = true;
		} 
		else if (inEqualBlock) {
			reportFile(files[i]);
			cout << endl;
			inEqualBlock = false;
		}
	}
	if (inEqualBlock) {
		reportFile(files[files.size() - 1]);
		cout << endl;
	}
}

// Main function
int main(int argc, char** argv) {
	FileList files = getAllFiles();
	selectionSortFileList(files);
	reportIdenticalFiles(files);
	return 0;
}