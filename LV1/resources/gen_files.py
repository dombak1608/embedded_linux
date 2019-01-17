for x in range(1,100):
   f = open("test_datoteka_"+str(x)+".txt", 'w')
   f.write("datoteka broj " + str(x) + "\n")
   f.close()