
f = open("./fleshka.img","rb")

for sector in range(0, 512) :
    if sector % 4 == 0 :
        print("Cluster \#"+str(int(sector/4)))
    print("Sector \#"+str(sector))
    for lines in range(0, 64) :
        arr = list(f.read(8))
        line =  ""
        for x in arr:
            print( hex(x) + ",", end=" ")
        print(" ")

f.close()

