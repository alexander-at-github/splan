#!/usr/bin/python

import math, os, random, re, subprocess, sys

#print("### argv len: ", len(sys.argv))
#for arg in sys.argv:
#    print('### arg: ', arg)

# The first argument can be the location of the splan executable.
splanCmd = './bin/splan'
if len(sys.argv) > 1:
    #print(sys.argv[1])
    splanCmd = sys.argv[1]
# The second argument can be a timeout value
timeout = '60' # seconds
if len(sys.argv) > 2:
    #print(sys.argv[2])
    timeout = sys.argv[2]

# The third argument can select an algorithm
algSelect = "-a"  # Defaults to A-Star Algorithm
if len(sys.argv) > 3:
    algSelect = sys.argv[3]

# The solution-found message from the planner
solutionFound = 'SOLUTION FOUND'

# The planner command. Could be changed or set for different planners.
#cmd = ['timeout', timeout, '/usr/bin/time', '-v',
#        splanCmd, '-d', 'domainDummy', '-p', 'problemDummy']

# Using linux control groups to limit memory usage. IT NEEDS TO BE SET UP
# BEFORE THIS SCRIPT IS RUN. How to:
# $ cgcreate -g memory:/myGroup
# Limit physical memory
# $ echo $(( 7 * 1024 * 1024 * 1024 )) > /sys/fs/cgroup/memory/myGroup/memory.limit_in_bytes
# Limit swap. This will limit the sum of physical memory + swap. You can not set this value
# lower than the physical memory limit. If you try to do so, you will get an error.
# $ echo $(( 0 )) > /sys/fs/cgroup/memory/myGroup/memory.memsw.limit_in_bytes
#
# Run a command:
# $ cgexec -g memory:myGroup cmdToRun
#
# ATTENTION: First boot with swapaccount=1 in kernel command line!
# ATTENTION: Must be run as root!

cmdPrefix = ['cgexec', '-g', 'memory:myGroup']

cmd = ['/usr/bin/time', '-v',
        splanCmd, '-d', 'domainDummy', '-p', 'problemDummy', '-t', timeout,
        algSelect]

cmd = cmdPrefix + cmd

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
    outF.write("  domain  |  problem  |  sol. found  |  termination  |  runtime [sec]  |  mem [kbytes] | " \
               "nodes expanded | number of ground actions | " \
               "variable occurrence | search depth or plan length | " \
               " k! * vo^k")
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
            assert all(re.match('P[0-9][0-9].*', file, re.IGNORECASE) for file in pddlFiles)
            domainFiles = []
            instanceFiles = []
            for pddlFile in pddlFiles:
                if re.match('.*DOMAIN.*', pddlFile, re.IGNORECASE):
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
        if algSelect == '-f': # fast-downward
            runFastDownward(domain, problem)
        else:
            runSimplePlan(domain, problem)

def runFastDownward(domain, problem):
    cmd = ['timeout', timeout,
           splanCmd, 'domainDummy', 'problemDummy', '--search', 'astar(lmcut())']
    cmd = cmdPrefix + cmd
    cmd = [elem if elem != 'domainDummy' else domain for elem in cmd]
    cmd = [elem if elem != 'problemDummy' else problem for elem in cmd]
    print(cmd)
    sys.stdout.flush()
    try:
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as ex:
        output = ex.output

    cmdOutStr = output.decode('utf-8')

    with open(logFN, 'a') as logF:
        logF.write('% ')
        logF.write(' '.join(cmd))
        logF.write('\n')
        logF.write(cmdOutStr)
        logF.write('\n\n')

    # The string with will be written to the .out (result) file
    writeOutStr = domain + ' ' + problem

    writeOutStr += ' S' if 'Solution found!' in cmdOutStr else ' -'

    if "terminated by signal" in cmdOutStr:
        sigNum = re.search("terminated by signal \d+", cmdOutStr)
        sigNum = sigNum.group(0)
        sigNum = [s for s in sigNum.split() if s.isdigit()][0]
        writeOutStr += ' sig' + str(sigNum)
    else:
        writeOutStr += ' norm'

    cmdOutLines = cmdOutStr.split('\n')

    time = None
    timeLines = [l for l in cmdOutLines if 'Total time: ' in l]
    if len(timeLines) > 0:
        assert len(timeLines) == 1
        time = re.findall("\d+", timeLines[0])
        time = time[0] if len(time) == 1 else (time[0] + '.' + time[1])
    else:
        assert len([l for l in cmdOutLines if 'caught signal' in l]) > 0
    writeOutStr += ' ' + (time if time else timeout) # If fast-downward does not give time, it was a timeout.

    mem = None
    memLines = [l for l in cmdOutLines if 'Peak memory: ' in l]
    if len(memLines) > 0:
        assert len(memLines) == 1
        mem = re.findall("\d+", memLines[0]) [0]
    writeOutStr += ' ' + (mem if mem else '-')

    # Find the number of nodes expanded
    numNdExp = None
    # Find last occurance of "nodes expanded".
    numNdExp = re.search(r"Evaluated \d+ state\(s\)", cmdOutStr)
    # Now numNdExp holds the last occurance of the deserved string.
    if numNdExp:
        numNdExp = numNdExp.group(0) # now it's a string
        numNdExp = [s for s in numNdExp.split() if s.isdigit()][0] # get first number
    #print(numNdExp)
    writeOutStr += ' ' + (numNdExp if numNdExp else '-')

    # The nest two parameters are not measured by fast-downward
    writeOutStr += ' - -'

    kk = re.search("Plan length: \d+ step\(s\)", cmdOutStr)
    if kk:
        kk = kk.group(0)
        kk = [s for s in kk.split() if s.isdigit()][0]
    writeOutStr += ' ' + (kk if kk else '-')

    # The next parameter is not measured by fast-downward
    writeOutStr += ' -'

    writeOutStr += '\n'

    with open(outFN, 'a') as outF: # Append to out file.
        outF.write(writeOutStr)



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

    cmdOutStr = output.decode('utf-8')
    #print(cmdOutStr)

    with open(logFN, 'a') as logF: # Append to log file
        logF.write('% ') # Signal that this line is a command
        logF.write(' '.join(cmdFinal))
        logF.write('\n')
        logF.write(cmdOutStr)
        logF.write('\n\n')

    # The string with will be written to the .out (result) file
    writeOutStr = domain + ' ' + problem

    writeOutStr += ' S' if solutionFound in cmdOutStr else ' -'

    if "terminated by signal" in cmdOutStr:
        sigNum = re.search("terminated by signal \d+", cmdOutStr)
        sigNum = sigNum.group(0)
        sigNum = [s for s in sigNum.split() if s.isdigit()][0]
        writeOutStr += ' sig' + str(sigNum)
    else:
        writeOutStr += ' norm'

    cmdOutLines = cmdOutStr.split('\n')

    time = None
    timeLines = [l for l in cmdOutLines if 'User time' in l]
    if len(timeLines) > 0:
        assert len(timeLines) == 1
        time = re.findall("\d+.\d+", timeLines[0]) [0]
    writeOutStr += ' ' + (time if time else '-')

    mem = None
    memLines = [l for l in cmdOutLines if 'Maximum resident set size' in l]
    if len(memLines) > 0:
        assert len(memLines) == 1
        mem = re.findall("\d+", memLines[0]) [0]
    writeOutStr += ' ' + (mem if mem else '-')

    # Find the number of nodes expanded
    numNdExp = None
    # Find last occurance of "nodes expanded".
    for numNdExp in re.finditer(r"nodes expanded: \d+", cmdOutStr):
        pass
    # Now numNdExp holds the last occurance of the deserved string.
    if numNdExp:
        numNdExp = numNdExp.group(0) # now it's a string
        numNdExp = [s for s in numNdExp.split() if s.isdigit()][0] # get first number
    #print(numNdExp)
    writeOutStr += ' ' + (numNdExp if numNdExp else '-')

    # Find number of ground actions in problem space
    numGrActs = re.search("number of ground actions in problem space: \d+", cmdOutStr)
    if numGrActs:
        numGrActs = numGrActs.group(0)
        numGrActs = [s for s in numGrActs.split() if s.isdigit()][0]
    #print(numGrActs)
    writeOutStr += ' ' + (numGrActs if numGrActs else '-')

    vo = re.search("variable occurrence: \d+", cmdOutStr)
    if vo:
        vo = vo.group(0)
        vo = [s for s in vo.split() if s.isdigit()][0]
    #print(vo)
    writeOutStr += ' ' + (vo if vo else '-')

    kk = re.search("solution length: \d+", cmdOutStr)
    if kk:
        kk = kk.group(0)
        kk = [s for s in kk.split() if s.isdigit()][0]
    writeOutStr += ' ' + (kk if kk else '-')

    cplxty = None
    if vo and kk:
        cplxty = str( math.factorial(int(kk)) * int(vo) ** int(kk) )
    writeOutStr += ' ' + (cplxty if cplxty else '-')

    writeOutStr += '\n'

    with open(outFN, 'a') as outF: # Append to out file.
        outF.write(writeOutStr)



def sorted_nicely( l ): 
    """ Sort the given iterable in the way that humans expect.""" 
    convert = lambda text: int(text) if text.isdigit() else text 
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
    return sorted(l, key = alphanum_key)

main()
