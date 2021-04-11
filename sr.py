PCT = 6
DST = 32000
BASE = 160000000/32

F0 = DST - DST * PCT/100
F1 = DST + DST * PCT/100

F0 = 25000
F1 = 100000

res = []

for i in range(1,64):
    for j in range(i,64):
        f = BASE / i / j
        # delta = (abs(DST-f) / DST) * 100
        if F0 <= f <= F1:
            res.append((f,i,j))

res.sort(key = lambda x: x[0])

for (f,i,j) in res:
    print(f"f = {f} [{i} {j}]")

flt = []
prevF = None
for (f,i,j) in res:
    if prevF == None or round(prevF,2) != round(f,2):
        flt.append((f,i,j))
    prevF = f

print()
print("Filtered")
for (f,i,j) in flt:
    print(f"f = {f} [{i} {j}]")

print("uint32_t f[] = {")
for (f,i,j) in flt:
    print(f" {int(f)},")
print("};")

print("uint8_t div1[] = {")
for (f,i,j) in flt:
    print(f" {i},")
print("};")

print("uint8_t div2[] = {")
for (f,i,j) in flt:
    print(f" {j},")
print("};")

print(f"const int CNT = {len(flt)};")
