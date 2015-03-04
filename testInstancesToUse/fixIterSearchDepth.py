#!/usr/bin/python

import re, math

with open('benchmark.out.4293159029644391793') as outFile, \
     open('benchmark.log.4293159029644391793') as logFile, \
     open('benchmark.out.4293159029644391793.fixed', 'w') as resultFile:

    outFileCS = outFile.read().split('\n')
    logFileC = logFile.read()

    # Copy the first two lines which are the header to the result file
    resultFile.write('\n'.join(outFileCS[:2]) + '\n')

    # Skip first two lines which are just header for the rest of the progam
    outFileCS = outFileCS[2:]
    #print(outFileCS[0])

    for line in outFileCS:
        #print(line)

        if not line:
            # The last line in the file is empty. That would cause an error
            continue

        lineSplit = line.split(' ')
        # Extract planning instance file name
        instanceName = lineSplit[1]
        #print(instanceName)

        # find last occurance of planning instance name in log-file

        # This approach is too slow
        #searchDepth = None
        #regexString = "depth search with depth (\d+).*" + instanceName
        #print(regexString)
        #regex = re.compile(regexString, re.DOTALL)
        ## Find last instance
        #for searchDepth in regex.finditer(logFileC):
        #    pass

        # Find first occurance of planning instance name in log file
        fileNameLoc = re.search(instanceName, logFileC)
        #fileNameLocIdx = fileNameLoc.start()
        #print(fileNameLocIdx)

        # Find search depth from here.
        logFileCTrimed = logFileC[fileNameLoc.start():]
        searchDepthOld = -1
        searchDepthNew = 0
        while searchDepthOld < searchDepthNew:
            searchDepthOld = searchDepthNew
            searchDepthNew = re.search(r"depth search with depth \d+", logFileCTrimed)
            if searchDepthNew:
                logFileCTrimed = logFileCTrimed[searchDepthNew.end():]
                searchDepthNew = searchDepthNew.group(0)
                searchDepthNew = [s for s in searchDepthNew.split() if s.isdigit()][0]
                searchDepthNew = int(searchDepthNew)
            else:
                searchDepthNew = -1
        searchDepth = searchDepthOld

        searchDepthStr = str(searchDepth)

        kk = searchDepth
        vo = int(lineSplit[8])
        k_times_vo_pow_k = str(math.factorial(kk) * vo ** kk)


        print(searchDepth)

        #print(lineSplit[:9])
        #print(lineSplit[10:])
        fixedLine = lineSplit[:9] + [searchDepthStr] + [k_times_vo_pow_k]
        fixedLine = ' '.join(fixedLine)
        #print('# line:')
        #print(line)
        #print('# fixed line:')
        #print(fixedLine)
        
        resultFile.write(fixedLine + '\n')



