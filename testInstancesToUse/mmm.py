with open("tmp.txt") as file:
    lines = file.readlines()
    ints = [int(i) for i in lines]
    ints.sort()
    print(ints)

    intsLenH = int(len(ints) / 2)
    if len(ints) % 2 == 0:
        med = (ints[intsLenH - 1] + ints[intsLenH]) / 2
    else:
        med = ints[intsLenH]
    print("{:d} \\newline {:f} \\newline {:d}".format(ints[0], med , ints[-1]))
