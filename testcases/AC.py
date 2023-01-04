for num in range(0,10):
    path = 'testcases\\' + str(num) + '.in'
    f = open(path,'r')
    a = int(0)
    b = int(0)
    a = int(f.readline())
    b = int(f.readline())
    
    # Function part
    a += b
    # End of function
    
    f.close()

    path = 'testcases\\' + str(num) + '.ans'
    f = open(path,'w')
    f.write(str(a) + '\n')
