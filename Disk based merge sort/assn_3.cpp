// CSC 541 Disk Based Merge Sorting
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sys/time.h>
using namespace std;

void performMergeBasedOnType(string mergeType, string inputFile, string outputFile);
void doBasicMerge(string inputFile, string outputFile);
void doMultistepMerge(string inputFile, string outputFile);
void doReplacementMerge(string inputFile, string outputFile);
void sortAndCreateRunFile(int arrKeyCnt, int *keyValues, int iterCnt, string inputFile, int lenOfRunCnt);
int *getInitialRuns(int runCnt, int bufSiz, string inputFile, bool isMultistep, int iterCnt, int prevRunCnt, int firstRunCnt);
int *reArrangeToAscendingHeap(int *heapArr, int n);
int *sift(int *heapArr, int i, int n);
void performReplacementSelection(string inputFile, string outputFile, int *heapArr, int heapCnt, int *bufArr, int bufCnt,
	int filePtr, int totKeyCnt);
string formatCount(int n, int len);
int lengthOfInteger(int n);
void mergeRunsAndWriteToOutputFile(int *runBuffer, int bufKeyCnt, int runCnt, string inputFile, string outputFile, bool isMultistep,
	int iterCnt, int maxBufKeyCnt, int initRunCnt, int prevRunCnt);
void writeBufferToRunFile(int *outputBuffer, int outputLen, int runNumber, string inputFile);
void copyFromFile(string runFile, string outputFile);

void copyFromFile(string runFile, string outputFile){
	ifstream run(runFile.c_str(), ios::in | ios::binary);
	run.seekg(0, ios::end);
	int eof = (int) run.tellg();
	int cnt = eof/sizeof(int);
	run.seekg(0, ios::beg);
	int *opBuffer = new int[cnt];
	for (int b = 0; b < cnt; b++) {
		run.read(reinterpret_cast<char*> (&opBuffer[b]), sizeof(int));
	}
	ofstream sort(outputFile.c_str(), ios::out | ios::binary);
	if (sort.is_open()) {
		sort.write(reinterpret_cast<char*>(opBuffer), cnt*sizeof(int));
	}
	delete[] opBuffer;
	run.close();
	sort.close();
}

int qsortComp(const void * x, const void * y) {
	return (*(int*)x - *(int*)y);
}

void performMergeBasedOnType(string mergeType, string inputFile, string outputFile) {
	if (mergeType.compare("--basic") == 0) {
		doBasicMerge(inputFile, outputFile);
	}
	else if (mergeType.compare("--multistep") == 0) {
		doMultistepMerge(inputFile, outputFile);
	}
	else if (mergeType.compare("--replacement") == 0) {
		doReplacementMerge(inputFile, outputFile);
	}
}

void doBasicMerge(string inputFile, string outputFile) {
	fstream fp(inputFile.c_str(), ios::in | ios::binary);
	if (fp.is_open()) {
		fp.seekg(0, ios::end);
		int len = (int)(fp.tellg());
		int totKeyCnt = len / sizeof(int);
		int maxBufKeyCnt = 1000;
		fp.seekg(0, ios::beg);
		int runCnt = 1;
		if (totKeyCnt > maxBufKeyCnt) {
			runCnt = totKeyCnt / maxBufKeyCnt;
			if ((totKeyCnt % maxBufKeyCnt) != 0) {
				runCnt = runCnt + 1;
			}
		}
		// start timer
		timeval start, end;
		gettimeofday(&start, NULL);
		// sort and create run files
		int minRunLen = 0;
		for (int a = 0; a < runCnt; a++) {
			int arrKeyCnt = maxBufKeyCnt;
			if (((totKeyCnt % maxBufKeyCnt) != 0) && (a == runCnt - 1)) {
				arrKeyCnt = totKeyCnt % maxBufKeyCnt;
			}
			int *keyBuffer = new int[arrKeyCnt];
			for (int b = 0; b < arrKeyCnt; b++) {
				fp.read(reinterpret_cast<char*> (&keyBuffer[b]), sizeof(int));
			}
			int lenOfRunCnt = lengthOfInteger(runCnt - 1);
			sortAndCreateRunFile(arrKeyCnt, keyBuffer, a, inputFile, lenOfRunCnt);
			if (a == 0) { minRunLen = arrKeyCnt; }
			else {
				if (arrKeyCnt < minRunLen) minRunLen = arrKeyCnt;
			}
		}
		fp.close();
		if (runCnt > 1) {
			int eachRunKeyCnt = maxBufKeyCnt / runCnt;
			int bufKeyCnt = maxBufKeyCnt;
			if ((eachRunKeyCnt * runCnt) != bufKeyCnt) bufKeyCnt = eachRunKeyCnt * runCnt;
			if (minRunLen < eachRunKeyCnt) bufKeyCnt = minRunLen * runCnt;
			int *runBuffer = getInitialRuns(runCnt, bufKeyCnt, inputFile, false, 0, 0, 0);
			mergeRunsAndWriteToOutputFile(runBuffer, bufKeyCnt, runCnt, inputFile, outputFile, false, 0, maxBufKeyCnt, 0, 0);
		}
		else {
			// write from input file to output file
			string runFile = inputFile + ".000";
			copyFromFile(runFile, outputFile);
		}
		// end timer and print
		gettimeofday(&end, NULL);
		double sec, microsec;
 		sec = (end.tv_sec - start.tv_sec);      // sec
    		microsec = abs(end.tv_usec - start.tv_usec); // ms 
    		cout << "Time: " << sec << "." << microsec << endl;
	}
	else {
		cout << "Unable to open the input file" << endl;
	}
}


void sortAndCreateRunFile(int arrKeyCnt, int *keyValues, int iterCnt, string inputFile, int lenOfRunCnt) {
	// sort key values using quick sort
	qsort(keyValues, arrKeyCnt, sizeof(int), qsortComp);
	string trail = formatCount(iterCnt, 3);
	string runFile = inputFile + "." + trail;
	// write sorted keys from buffer to file
	fstream ixfp(runFile.c_str(), ios::out | ios::binary);
	if (ixfp.is_open()) {
		ixfp.write(reinterpret_cast<char*>(keyValues), arrKeyCnt*sizeof(int));
	}
	delete[] keyValues;
	ixfp.close();
}

int *getInitialRuns(int runCnt, int bufKeyCnt, string inputFile, bool isMultistep, int iterCnt, int prevRunCnt, int firstRunCnt) {
	int eachRunKeyCnt = bufKeyCnt / runCnt;
	int *runBuffer = new int[bufKeyCnt];
	for (int b = 0; b < runCnt; b++) {
		int lenOfRunCnt = lengthOfInteger(runCnt - 1);
		string trail = formatCount(b, 3);
		string runFile = inputFile + "." + trail;
		if (isMultistep) {
			int lenOfRunCnt = lengthOfInteger(firstRunCnt - 1);
			string trail = formatCount((prevRunCnt*iterCnt) + b, 3);
			runFile = inputFile + "." + trail;
		}
		fstream run(runFile.c_str(), ios::in | ios::binary);
		if (run.is_open()) {
			run.seekg(0, ios::end);
			int eofPos = (int) run.tellg();
			int keyNos = eofPos / sizeof(int);
			run.seekg(0, ios::beg);
			int init = eachRunKeyCnt*b;
			int cond = (b + 1)*eachRunKeyCnt;
			if(keyNos < eachRunKeyCnt) cond = (eachRunKeyCnt*b) + keyNos;
			for (int x = init; x < cond; x++) {
				int curPos = (int)run.tellg();
				if (curPos < eofPos) {
					run.read(reinterpret_cast<char*>(&runBuffer[x]), sizeof(int));
				}
			}
		}
		run.close();
	}
	return runBuffer;
}

int getFirstWinnerIndex(int runCnt, int *remCntArr) {
	int v = -1;
	for (v = 0; v < runCnt; v++) {
		if (remCntArr[v] > 0) {
			break;
		}
	}
	if (v == runCnt) v = -1;
	return v;
}

void mergeRunsAndWriteToOutputFile(int *runBuffer, int bufKeyCnt, int runCnt, string inputFile, string outputFile, bool isMultistep,
	int iterCnt, int maxBufKeyCnt, int initRunCnt, int prevRunCnt) {
	int eachRunKeyCnt = bufKeyCnt / runCnt;
	int *posArr = new int[runCnt];	
	int *remCntArr = new int[runCnt];
	int *runFilePtrArr = new int[runCnt];
	// auxiliary arrays
	for (int i = 0; i < runCnt; i++) {
		posArr[i] = i*eachRunKeyCnt;
	}
	for (int q = 0; q < runCnt; q++) {
		remCntArr[q] = eachRunKeyCnt;
	}
	int runFilePtrPos = eachRunKeyCnt * sizeof(int);
	for (int p = 0; p < runCnt; p++) {
		runFilePtrArr[p] = runFilePtrPos;
	}
	while (getFirstWinnerIndex(runCnt, remCntArr) != -1) {
		int *finalArr = new int[maxBufKeyCnt];
		int actualjVal = maxBufKeyCnt;
		for (int j = 0; j < maxBufKeyCnt; j++) {
			int winIdx = getFirstWinnerIndex(runCnt, remCntArr);
			if (winIdx != -1) {
				int winner = runBuffer[posArr[winIdx]];
				int idx = winIdx;
				for (int k = (winIdx + 1); k < runCnt; k++) {
					if ((remCntArr[k] > 0) && (posArr[k] < bufKeyCnt)) {
						if (runBuffer[posArr[k]] < winner) {
							winner = runBuffer[posArr[k]];
							idx = k;
						}
					}
				}
				finalArr[j] = winner;
				remCntArr[idx] = remCntArr[idx] - 1;
				//check if run is depleted before advancing the pointer
				if (remCntArr[idx] == 0) {
					// load next set of keys from the file	
					int lenOfRunCnt = lengthOfInteger(runCnt - 1);
					string trail = formatCount(idx, 3);
					string runFile = inputFile + "." + trail;
					if (isMultistep) {
						int lenOfRunCnt = lengthOfInteger(initRunCnt - 1);
						string trail = formatCount((iterCnt*prevRunCnt) + idx, 3);
						runFile = inputFile + "." + trail;
					}
					fstream run(runFile.c_str(), ios::in | ios::binary);

					run.seekg(0, ios::end);
					int eofPos = (int)run.tellg();
					int ptrPos = runFilePtrArr[idx];

					if (ptrPos < eofPos) {
						run.seekg(ptrPos, ios::beg);
						if (run.is_open()) {
							int init = eachRunKeyCnt*idx;
							int cond = (idx + 1)*eachRunKeyCnt;
							for (int x = init; x < cond; x++) {
								int curPos = (int)run.tellg();
								if (curPos < eofPos) {
									run.read(reinterpret_cast<char*>(&runBuffer[x]), sizeof(int));
									remCntArr[idx] = remCntArr[idx] + 1;
								}
								else {
									break;
								}
							}
							runFilePtrArr[idx] = (int)run.tellg();
						}
					}
					run.close();
					posArr[idx] = eachRunKeyCnt * idx;    //reset posArr to first index
				}
				else {
					posArr[idx] += 1;
				}
			}
			else {
				actualjVal = j;
				break;
			}
		} // inner for loop ends

		fstream ofp;
		ofp.open(outputFile.c_str(), ios::in | ios::out | ios::binary);			//writing chunks to output file
		if (ofp.is_open()) {
			ofp.seekg(0, ios::end);
			ofp.write(reinterpret_cast<char*>(finalArr), actualjVal*sizeof(int));
		}
		ofp.close();
		delete[] finalArr;
	} //while loop
	delete[] runBuffer;
	delete[] posArr;
	delete[] runFilePtrArr;
	delete[] remCntArr;
}

void doMultistepMerge(string inputFile, string outputFile) {
	fstream fp(inputFile.c_str(), ios::in | ios::binary);
	if (fp.is_open()) {
		fp.seekg(0, ios::end);
		int len = (int)(fp.tellg());
		int totKeyCnt = len / sizeof(int);
		int maxBufKeyCnt = 1000;
		fp.seekg(0, ios::beg);
		int runCnt = 1;
		if (totKeyCnt > maxBufKeyCnt) {
			runCnt = totKeyCnt / maxBufKeyCnt;
			if ((totKeyCnt % maxBufKeyCnt) != 0) {
				runCnt = runCnt + 1;
			}
		}
		// start timer
		timeval start, end;
		gettimeofday(&start, NULL);
		// sort and create intermediate run files
		int minRunLen = 0;
		for (int a = 0; a < runCnt; a++) {
			int arrKeyCnt = maxBufKeyCnt;
			if (((totKeyCnt % maxBufKeyCnt) != 0) && (a == runCnt - 1)) {
				arrKeyCnt = totKeyCnt % maxBufKeyCnt;
			}
			int *keyBuffer = new int[arrKeyCnt];
			for (int b = 0; b < arrKeyCnt; b++) {
				fp.read(reinterpret_cast<char*> (&keyBuffer[b]), sizeof(int));
			}
			int lenOfRunCnt = lengthOfInteger(runCnt - 1);
			sortAndCreateRunFile(arrKeyCnt, keyBuffer, a, inputFile, lenOfRunCnt);
			if (a == 0) { minRunLen = arrKeyCnt; }
			else {
				if (arrKeyCnt < minRunLen) minRunLen = arrKeyCnt;
			}
		}
		fp.close();
		float interimRunCnt = 15;
		int interimKeysPerRun = (int)(maxBufKeyCnt / interimRunCnt);
		int actualBufKeyCnt = (int)(interimKeysPerRun * interimRunCnt);
		if (runCnt >= 15) {
			int superRunCnt = (int)ceil(runCnt / interimRunCnt);
			for (int b = 0; b < superRunCnt; b++) {
				int actualRunCnt = (int)interimRunCnt;
				if ((b == (superRunCnt - 1)) && ((runCnt / interimRunCnt) < superRunCnt)) {
					actualRunCnt = runCnt % actualRunCnt;
				}

				int actualKeysPerRun = maxBufKeyCnt / actualRunCnt;	
				int actBufKeyCnt = actualKeysPerRun * actualRunCnt;
				int *runBuffer = getInitialRuns(actualRunCnt, actBufKeyCnt, inputFile, true, b, (int)interimRunCnt, runCnt);
				int lenOfRunCnt = lengthOfInteger(superRunCnt - 1);
				string trail = formatCount(b, 3);
				string superRunFile = inputFile + ".super." + trail;
				//create the super run file
				fstream srf(superRunFile.c_str(), ios::out);
				srf.close();
				mergeRunsAndWriteToOutputFile(runBuffer, actBufKeyCnt, actualRunCnt, inputFile, superRunFile, true, b, maxBufKeyCnt, runCnt, (int)interimRunCnt);
			}
			// final merge of super runs
			string inputSuperFile = inputFile + ".super";
			int eachRunKeyCnt = maxBufKeyCnt / superRunCnt;
			int bufKeyCnt = maxBufKeyCnt;
			if ((eachRunKeyCnt * superRunCnt) != bufKeyCnt) bufKeyCnt = eachRunKeyCnt * superRunCnt;
			int *runBuffer = getInitialRuns(superRunCnt, bufKeyCnt, inputSuperFile, false, 0, 0, 0);
			mergeRunsAndWriteToOutputFile(runBuffer, bufKeyCnt, superRunCnt, inputSuperFile, outputFile, false, 0, maxBufKeyCnt, 0, 0);
		}
		else {
			if (runCnt > 1) {
				int eachRunKeyCnt = maxBufKeyCnt / runCnt;
				int bufKeyCnt = maxBufKeyCnt;
				if ((eachRunKeyCnt * runCnt) != bufKeyCnt) bufKeyCnt = eachRunKeyCnt * runCnt;
				if (minRunLen < eachRunKeyCnt) bufKeyCnt = minRunLen * runCnt;
				int *runBuffer = getInitialRuns(runCnt, bufKeyCnt, inputFile, false, 0, 0, 0);
				// create super run file
				string superRunFile = inputFile + ".super.000";
				fstream srf(superRunFile.c_str(), ios::out);
				srf.close();
				mergeRunsAndWriteToOutputFile(runBuffer, bufKeyCnt, runCnt, inputFile, superRunFile, false, 0, maxBufKeyCnt, 0, 0);
				// copy to final output File
				copyFromFile(superRunFile, outputFile);
			}
			else {
				// write from input file to output file
				string runFile = inputFile + ".000";
				copyFromFile(runFile, outputFile);
			}
		}
		// end timer and print
		gettimeofday(&end, NULL);
		double sec, microsec;
 		sec = (end.tv_sec - start.tv_sec);      // sec
    		microsec = abs(end.tv_usec - start.tv_usec); // ms
    		cout << "Time: " << sec << "." << microsec << endl;
	}
	else {
		cout << "Unable to open the input file" << endl;
	}
}



void doReplacementMerge(string inputFile, string outputFile) {
	fstream fp(inputFile.c_str(), ios::in | ios::binary);
	if (fp.is_open()) {
		fp.seekg(0, ios::end);
		int len = (int)(fp.tellg());
		int totKeyCnt = len / sizeof(int);
		int heapCnt = 750;
		int bufCnt = 250;
		fp.seekg(0, ios::beg);
		// read bufSiz contents from input file
		if (totKeyCnt > heapCnt) {
			// read buffer into array
			int *heapArr = new int[heapCnt];
			for (int a = 0; a < heapCnt; a++) {
				fp.read(reinterpret_cast<char*> (&heapArr[a]), sizeof(int));
			}
			if (totKeyCnt < 1000) bufCnt = totKeyCnt - heapCnt;
			int *bufArr = new int[bufCnt];
			for (int b = 0; b < bufCnt; b++) {
				fp.read(reinterpret_cast<char*> (&bufArr[b]), sizeof(int));
			}
			int filePtrPos = (int)fp.tellg();
			fp.close();
			performReplacementSelection(inputFile, outputFile, heapArr, heapCnt, bufArr, bufCnt, filePtrPos, totKeyCnt);
		}
		else {
			// start timer
			timeval start, end;
			gettimeofday(&start, NULL);
			// sort a create one run file and one output file
			// read buffer into array
			int *heapArr = new int[totKeyCnt];
			for (int a = 0; a < totKeyCnt; a++) {
				fp.read(reinterpret_cast<char*> (&heapArr[a]), sizeof(int));
			}
			qsort(heapArr, totKeyCnt, sizeof(int), qsortComp);

			string runFile = inputFile + ".000";
			// write sorted keys from buffer to file
			fstream ixfp(runFile.c_str(), ios::out | ios::binary);
			if (ixfp.is_open()) {
				ixfp.write(reinterpret_cast<char*>(heapArr), totKeyCnt*sizeof(int));
			}

			// write sorted keys from buffer to file
			fstream ofp(outputFile.c_str(), ios::out | ios::binary);
			if (ofp.is_open()) {
				ofp.write(reinterpret_cast<char*>(heapArr), totKeyCnt*sizeof(int));
			}
			delete[] heapArr;
			ixfp.close();
			ofp.close();
			fp.close();

			// end timer and print
			gettimeofday(&end, NULL);
			double sec, microsec;
	 		sec = (end.tv_sec - start.tv_sec);      // sec
	    		microsec = abs(end.tv_usec - start.tv_usec); // ms
	    		cout << "Time: " << sec << "." << microsec << endl;
		}
	}
	else {
		cout << "Unable to open the input file" << endl;
	}
}

void performReplacementSelection(string inputFile, string outputFile, int *heapArr, int heapCnt, int *bufArr, int bufCnt, int filePtr, int totKeyCnt) {
	int initBufCnt = bufCnt;
	int initHeapCnt = heapCnt;
	int secHeapCnt = 0;
	int opBufCnt = 1000;
	int curBufArrIdx = 0;
	int runNumber = 0;
	int minRunLen = 0;

	// start timer
	timeval start, end;
	gettimeofday(&start, NULL);

	int lastRunKeyCnt = 0;
	while (heapCnt > 0) {
		if ((lastRunKeyCnt > 0) && (lastRunKeyCnt < initHeapCnt)) {
			int actualSecKeyCnt = initHeapCnt - lastRunKeyCnt;
			int j = lastRunKeyCnt;
			for (int i = 0; i < actualSecKeyCnt; i++) {
				heapArr[i] = heapArr[j];
				j++;
			}
		}
		heapArr = reArrangeToAscendingHeap(heapArr, heapCnt);
		int totalKeysWritten = 0;
		// create run file
		string trail = formatCount(runNumber, 3);
		string runFile = inputFile + "." + trail;
		fstream fp(runFile.c_str(), ios::out);
		fp.close();
		while (heapCnt > 0) {
			int *outputBuffer = new int[opBufCnt];
			int j = 0;
			for (j = 0; j < opBufCnt; j++) {
				if ((heapCnt > 0) && (curBufArrIdx != -1) && (curBufArrIdx < bufCnt)) {
					int minHeapElement = heapArr[0];
					outputBuffer[j] = minHeapElement;
					if (minHeapElement <= bufArr[curBufArrIdx]) {
						heapArr[0] = bufArr[curBufArrIdx];
					}
					else {
						heapArr[0] = heapArr[heapCnt-1];
						heapArr[heapCnt-1] = bufArr[curBufArrIdx];
						heapCnt = heapCnt - 1;
						secHeapCnt = secHeapCnt + 1;
					}
					curBufArrIdx = curBufArrIdx + 1;
					if (curBufArrIdx == initBufCnt) {
						// load next set of keys into buffer - if eof is reached, then done!
						fstream ipf(inputFile.c_str(), ios::in | ios::binary);
						if (ipf.is_open()) {
							ipf.seekg(0, ios::end);
							int eofPos = (int)ipf.tellg();
							ipf.seekg(filePtr, ios::beg);
							for (int x = 0; x < initBufCnt; x++) {
								int curPos = (int)ipf.tellg();
								if (curPos < eofPos) {
									ipf.read(reinterpret_cast<char*>(&bufArr[x]), sizeof(int));
								}
								else {
									bufCnt = x;
									break;
								}
							}
							filePtr = (int)ipf.tellg();
							curBufArrIdx = (bufCnt == 0) ? -1 : 0;
							ipf.close();
						}
					}
					else if (curBufArrIdx == bufCnt) {
						curBufArrIdx = -1;
						bufCnt = 0;
					}
				}
				else {
					if (heapCnt > 0) {
						lastRunKeyCnt += 1;
						int minHeapElement = heapArr[0];
						outputBuffer[j] = minHeapElement;
						heapArr[0] = heapArr[heapCnt-1];
						heapCnt = heapCnt - 1;
					}
					else {
						break;
					}
				}
				heapArr = reArrangeToAscendingHeap(heapArr, heapCnt);
			} // for loop
			int outputLen = j;
			// current run is not completed but output buffer is full
			writeBufferToRunFile(outputBuffer, outputLen, runNumber, runFile);
			totalKeysWritten += outputLen;
		} // inner while
		if ((heapCnt == 0) && (secHeapCnt > 0)) {
			heapCnt = secHeapCnt;
			secHeapCnt = 0;
		}
		if (runNumber == 0) {
			minRunLen = totalKeysWritten;
		}
		else {
			if (totalKeysWritten < minRunLen) minRunLen = totalKeysWritten;
		}
		// current run is completed
		runNumber += 1;	
	}
	delete[] heapArr;
	delete[] bufArr;
	
	int eachRunKeyCnt = opBufCnt / runNumber;
	int bufKeyCnt = opBufCnt;
	if ((eachRunKeyCnt * runNumber) != bufKeyCnt) bufKeyCnt = eachRunKeyCnt * runNumber;
	if (minRunLen < eachRunKeyCnt) bufKeyCnt = minRunLen * runNumber;
	int *runBuffer = getInitialRuns(runNumber, bufKeyCnt, inputFile, false, 0, 0, 0);
	mergeRunsAndWriteToOutputFile(runBuffer, bufKeyCnt, runNumber, inputFile, outputFile, false, 0, opBufCnt, 0, 0);
	// end timer and print
	gettimeofday(&end, NULL);
	double sec, microsec;
 	sec = (end.tv_sec - start.tv_sec);      // sec
    	microsec = abs(end.tv_usec - start.tv_usec); // ms
    	cout << "Time: " << sec << "." << microsec << endl;
}

void writeBufferToRunFile(int *outputBuffer, int outputLen, int runNumber, string runFile) {
	// write sorted keys from buffer to file
	fstream fp(runFile.c_str(), ios::out | ios::app | ios::binary);
	if (fp.is_open()) {
		fp.write(reinterpret_cast<char*>(outputBuffer), outputLen*sizeof(int));
		fp.close();
	}
	delete[] outputBuffer;
}

int *reArrangeToAscendingHeap(int *heapArr, int n) {
	// constructs an asc heap and returns array-- min-heap
	int i = (int)floor(n / 2) - 1;
	while (i >= 0) {
		heapArr = sift(heapArr, i, n);
		i--;
	}
	return heapArr;
}

int *sift(int *heapArr, int i, int n) {
	while (i <= (floor(n / 2)-1)) {
		int j = (2 * i) + 1;
		int k = j + 1;
		int minIdx;
		if ((k < n) && heapArr[k] <= heapArr[j]) {
			minIdx = k;
		}
		else {
			minIdx = j;
		}
		if (heapArr[i] <= heapArr[minIdx]) {
			break;
		}
		// swap i and minIdx elements
		int temp = heapArr[i];
		heapArr[i] = heapArr[minIdx];
		heapArr[minIdx] = temp;
		i = minIdx;
	}
	return heapArr;
}

string formatCount(int n, int runLen) {
	int length = lengthOfInteger(n);
	if (n == 0) length = 1;
	if (length == runLen) {
		return static_cast<ostringstream*>(&(ostringstream() << n))->str();
	}
	else {
		int dif = runLen - length;
		string str = "";
		while (dif > 0) {
			str = str + "0";
			dif--;
		}
		str = str + static_cast<ostringstream*>(&(ostringstream() << n))->str();
		return str;
	}
}

int lengthOfInteger(int n)
{
	int cnt = 0;
	while (n) {
		n = n / 10;
		cnt++;
	}
	return cnt;
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		cout << "\nEnter the right number of parameters of the format: mergesort-method index-file sorted-index-file" << endl;
	}
	else {
		string mergeSortMethod = argv[1];
		string indexFileName = argv[2];
		string sortedIndexFile = argv[3];
		ifstream file(indexFileName.c_str());
		if (!file) {
			cout << "Unable to open the input file" << endl;
		}
		else {
			fstream ofp(sortedIndexFile.c_str(), ios::out);
			ofp.close();
			if (mergeSortMethod.compare("--basic") == 0 || mergeSortMethod.compare("--multistep") == 0
				|| mergeSortMethod.compare("--replacement") == 0) {
				performMergeBasedOnType(mergeSortMethod, indexFileName, sortedIndexFile);
			}
			else {
				cout << "\nEnter a valid merge sort method" << endl;
			}
		}
	}
	return 0;
}



