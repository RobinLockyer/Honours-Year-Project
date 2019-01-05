from random import randint

minVal = 0
maxVal = 200

def countInversions(arr):

    if len(arr) == 1:
        return 1
    
    mid = len(arr)//2
    a = arr[:mid]
    b = arr[mid:]

    inversions = countInversions(a) + countInversions(b)

    aCounter = 0
    bCounter = 0
    cCounter = 0
    
    while aCounter < len(a) and bCounter < len(b):
        if a < b:
            arr[cCounter] = a[aCounter]
            aCounter+=1
        else:
            arr[cCounter] = b[bCounter]
            bCounter+=1
            inversions+=1
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

def randArr(size):
    arr = []
    for i in range(size):
        arr.append(randint(minVal, maxVal+1))
    return arr

def main():
    print(countInversions([7,6,5,4,3,2,1]))

if __name__ == "__main__":
    main()
