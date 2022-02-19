xaxis = {"A":1, "B":2,"C":3,"D":4,"E":5,"F":6,"G":7,"H":8}          #dictionary for converting alphabets to numbers
position = input("Enter the Knight Position:")
x_pos = int(xaxis.get(position[0]))
y_pos = int(position[1])

#function for possible knight movements

def move(x_pos, y_pos):

    Letter = {1:"A", 2:"B",3:"C",4:"D",5:"E",6:"F",7:"G",8:"H"}    #dictionary for converting numbers back to alphabets
    Listofpos=[]                                                    #empty position list

    def numtoletters(pos):                                          #function for converting numbers back to alphabets
        Move=Letter[pos[0]]+str(pos[1])
        return Move

    #maxinmim eight possible moves
    #1
    one=[x_pos-1,y_pos+2]
    if one[0]<1 or one[0]>8 or one[1]<1 or one[1]>8:
        pass
    else:
        one= numtoletters(one)
        Listofpos.append(one)
    #2
    two=[x_pos+1,y_pos+2]
    if two[0]<1 or two[0]>8 or two[1]<1 or two[1]>8:
        pass
    else:
        two= numtoletters(two)
        Listofpos.append(two)
    #3
    three=[x_pos+2,y_pos+1]
    if three[0]<1 or three[0]>8 or three[1]<1 or three[1]>8:
        pass
    else:
        three= numtoletters(three)
        Listofpos.append(three)
    #4
    four=[x_pos+2,y_pos-1]
    if four[0]<1 or four[0]>8 or four[1]<1 or four[1]>8:
        pass
    else:
        four= numtoletters(four)
        Listofpos.append(four)
    #5
    five=[x_pos-1,y_pos-2]
    if five[0]<1 or five[0]>8 or five[1]<1 or five[1]>8:
        pass
    else:
        five= numtoletters(five)
        Listofpos.append(five)
    #6
    six=[x_pos+1,y_pos-2]
    if six[0]<1 or six[0]>8 or six[1]<1 or six[1]>8:
        pass
    else:
        six= numtoletters(six)
        Listofpos.append(six)
    #7
    seven=[x_pos-2,y_pos+1]
    if seven[0]<1 or seven[0]>8 or seven[1]<1 or seven[1]>8:
        pass
    else:
        seven= numtoletters(seven)
        Listofpos.append(seven)
    #8
    eight=[x_pos-2,y_pos-1]
    if eight[0]<1 or eight[0]>8 or eight[1]<1 or eight[1]>8:
        pass
    else:
        eight= numtoletters(eight)
        Listofpos.append(eight)

    final = print("Legal Moves:",Listofpos)
    return Listofpos


Listofpos = move(x_pos, y_pos)     #function
testpos = input("Enter the position of a square: ")

#Conclusion
if testpos in Listofpos:
    print(testpos,"is a valid square for the knight on ",position)
else:
    print(testpos,"is an invalid square for the knight on ", position)

    


