# Generate 2 big number
import random

def generate():
    b = random.randint(2,3)
    ans = str()

    if(b % 2): # Test for negative number
        ans = '-'

    while b:
        ans += str(random.randint(1,1e5))
        b -= 1
    return ans

for num in range(0,10):
    path = 'testcases\\' + str(num) + '.in'
    f = open(path,'w')
    f.write(generate() + '\n')
    f.write(generate() + '\n')
    f.close()
