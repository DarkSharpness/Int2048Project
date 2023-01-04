
for num in range(0,10):
    path = 'testcases\\' + str(num) + '.in'
    f = open(path,'r');
    a = int(f.readline())
    b = int(f.readline())

    # Function part
    a += b
    # End of function

    f.close()
    path = 'testcases\\' + str(num) + '.ans'
    f = open(path,'r')

    b = int(f.readline())

    if(a != b):
        print('Wrong Answer at testpoint',num)
    else:
        print('Accepted at testpoint',num)
