from random import randint
from sys import argv

minIntVal = 0
maxIntVal = 400

minArrSize = 2
maxArrSize = 100

numTests = 40
maxOps = 3000

def countInversions(arr):

    if len(arr) <= 1:
        return 0
    
    mid = len(arr)//2
    a = arr[:mid]
    b = arr[mid:]

    inversions = countInversions(a) + countInversions(b)

    aCounter = 0
    bCounter = 0
    cCounter = 0
    
    while aCounter < len(a) and bCounter < len(b):
        if a[aCounter] < b[bCounter]:
            arr[cCounter] = a[aCounter]
            aCounter+=1
        else:
            arr[cCounter] = b[bCounter]
            bCounter+=1
            inversions+=(len(a)-aCounter)
            
        cCounter+=1

    if aCounter < len(a):
        for i in range(aCounter, len(a)):
            arr[cCounter] = a[i]
            cCounter+=1
    else:
        for i in range(bCounter, len(b)):
            arr[cCounter] = b[i]
            cCounter+=1
            
    return inversions    

def randArr():
    arr = []
    for i in range(randint(minArrSize,maxArrSize+1)):
        arr.append(randint(minIntVal, maxIntVal+1))
    return arr

def main():
    #print(countInversions([6,7,5,4,3,1,2]))
    file = open(argv[1],"w+")
    outString = ""
    for testSet in range(maxOps):
        for test in range(numTests):
            arr = randArr()
            outString += str(len(arr)) + " "+ str(countInversions(arr.copy())) + " " + str(arr).strip('[]').replace(",","") + "\n"
        outString += "\n"
    file.write(outString)

if __name__ == "__main__":
    main()
