#!/usr/bin/python

import os, random, re, subprocess, sys

#print("### argv len: ", len(sys.argv))
#for arg in sys.argv:
#    print('### arg: ', arg)

# The first argument can be the location of the splan executable.
splanCmd = './bin/splan'
if len(sys.argv) > 1:
    print(sys.argv[1])
    splanCmd = sys.argv[1]
# The second argument can be a timeout value
timeout = '60' # seconds
if len(sys.argv) > 2:
    print(sys.argv[2])
    timeout = sys.argv[2]

# The solution-found message from the planner
solutionFound = 'SOLUTION FOUND'

# The planner command. Could be changed or set for different planners.
#cmd = ['timeout', timeout, '/usr/bin/time', '-v',
#        splanCmd, '-d', 'domainDummy', '-p', 'problemDummy']
# When we swap the order of 'time' and 'timeout' multiple processes will be run
# run at the same time. Why?
cmd = ['/usr/bin/time', '-v', 'timeout', timeout,
        splanCmd, '-d', 'domainDummy', '-p', 'problemDummy']

# With argument None uses system time.
random.seed(None)


uuid = str(random.randint(0, sys.maxsize))
# Out file for results.
outFN = 'benchmark.out.' + uuid
# Out file for log.
logFN = 'benchmark.log.' + uuid

# Write header to outfile.
with open(outFN, "a") as outF:
    outF.write("timeout [sec]: ")
    outF.write(timeout)
    outF.write("\n")
    outF.write("  domain  |  problem  |  runtime [sec]  |  mem [kbytes]")
    outF.write("\n")

def main():
    for root, subFolders, filesOriginal in os.walk(os.getcwd()):
        files = filesOriginal

        # Ignore the files we don't want.
        ignoreFileName = '.ignore'
        if any(re.match(ignoreFileName, file) for file in files):
            with open(os.path.join(root, ignoreFileName)) as ignoreF:
                ignoreLines = ignoreF.readlines()
                ignoreLines.append(ignoreFileName)
                for line in ignoreLines:
                    line = line.rstrip()
                    #print(line)
                    try:
                        files.remove(line)
                    except ValueError:
                        #print("Value Error: ", line)
                        pass

        # Get all files with .pddl suffix
        regex = re.compile('.*\.pddl', re.IGNORECASE)
        pddlFiles = [file for file in files if regex.match(file)]
        numOfPddlFiles = len(pddlFiles)

        if numOfPddlFiles < 1:
            #print("No *.pddl file(s) in this directory")
            continue

        #print(root)
        #print(subFolders)
        #print(files)

        #print("number of *.pddl files: ", numOfPddlFiles)

        ## Variable for the problem domain
        # If 'domain.pddl' exists, then use it as domain, and remove it from
        # the list of pddl-files.
        domain = pddlFiles.pop(pddlFiles.index('domain.pddl')) \
                    if 'domain.pddl' in pddlFiles \
                    else None
        domain = pddlFiles.pop(pddlFiles.index('DOMAIN.PDDL')) \
                    if 'DOMAIN.PDDL' in pddlFiles \
                    else domain
        #print("domain: ", domain)

        # A list of tuples. The first element of the tuple must be a domain and
        # the second element a problem.
        domProbTuples = []

        if domain != None:
            # There exists a domain.pddl or DOMAIN.PDDL
            pddlFiles = sorted_nicely(pddlFiles)
            for pInstance in pddlFiles:
                domProbTuples.append( (domain, pInstance) )

        elif numOfPddlFiles == 1:
            # There exists only a single pddl file. It must be the domain
            # definition. The problem files don't have a pddl suffix then.
            domain = pddlFiles[0]
            #print("domain: ", domain)
            files.remove(domain)
            #print(files)
            files = sorted_nicely(files)
            for pInstance in files:
                domProbTuples.append( (domain, pInstance) )

        else:
            assert all(re.match('P[0-9][0-9].*', file) for file in pddlFiles)
            domainFiles = []
            instanceFiles = []
            for pddlFile in pddlFiles:
                if re.match('.*DOMAIN.*', pddlFile):
                    domainFiles.append(pddlFile)
                else:
                    instanceFiles.append(pddlFile)
            #print("domain size: ", len(domainFiles),
            #      " instance size: ", len(instanceFiles))
            domainFiles = sorted_nicely(domainFiles)
            for domain in domainFiles:
                prefix = domain[:3]
                #print(prefix)
                regex = re.compile(prefix + '.*')
                instancesForDomain = [d for d in instanceFiles \
                                                 if regex.match(d)]
                instancesForDomain = sorted_nicely(instancesForDomain)
                for instance in instancesForDomain:
                    domProbTuples.append( (domain, instance) )

        # Make complete paths for the input files.
        domProbTuplesNew = []
        for dom, prob in domProbTuples:
            domJ = os.path.join(root, dom)
            probJ = os.path.join(root, prob)
            domProbTuplesNew.append( (domJ, probJ) );
        domProbTuples = domProbTuplesNew
        runSimplePlanArr(domProbTuples)

def runSimplePlanArr(array):
    for domain, problem in array:
        runSimplePlan(domain, problem)

def runSimplePlan(domain, problem):
    #cmd = ['timeout', timeout, '/usr/bin/time', '-v',
    #        splanCmd, '-d', domain, '-p', problem]
    # Replace domainDummy and ProblemDummy
    cmdFinal = [domain if elem == "domainDummy" else elem for elem in cmd]
    cmdFinal = [problem if elem== "problemDummy" else elem for elem in cmdFinal]

    print(cmdFinal)
    sys.stdout.flush()
    try:
        output = subprocess.check_output(cmdFinal, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as ex:
        # Non zero return code. That is, timeout happend.
        # Just ignore
        output = ex.output
        pass

    outputStr = output.decode('utf-8')

    with open(logFN, 'a') as logF: # Append to log file
        logF.write('% ') # Signal that this line is a command
        logF.write(' '.join(cmdFinal))
        logF.write('\n')
        logF.write(outputStr)
        logF.write('\n\n')

    # Set time an memusage to empty first and then, if the planner did solve
    # the problem, to the actual number.
    time = '-'
    mem = '-'
    if solutionFound in outputStr:
        outLines = outputStr.split('\n')

        timeLine = [l for l in outLines if 'User time' in l]
        assert len(timeLine) == 1
        # Isolate floating point number from string, which gives time.
        time = re.findall("\d+.\d+", timeLine[0]) [0]

        memLine = [l for l in outLines if 'Maximum resident set size' in l]
        assert len(memLine) == 1
        mem = re.findall("\d+", memLine[0]) [0]

    # Write measured numbers to out-file.
    with open(outFN, 'a') as outF: # Append to out file.
        strToWrite = domain + ' ' + problem + ' ' + time + ' ' + mem + '\n'
        outF.write(strToWrite)

def sorted_nicely( l ): 
    """ Sort the given iterable in the way that humans expect.""" 
    convert = lambda text: int(text) if text.isdigit() else text 
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
    return sorted(l, key = alphanum_key)

main()