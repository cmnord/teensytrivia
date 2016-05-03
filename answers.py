import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())
print( "Content-type:text/html\r\n\r\n")
print('<html>')

cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')
method_type = get_method_type()
form = cgi.FieldStorage() #get specified parameters!

def insertIntoDB(gameID,roundNum,questionID,sender,deviceType,delta,isCorrect):
    cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')
    query = ("SELECT * FROM response_db WHERE sender=\'"+ str(sender)+"\'")
    cnx.query(query)
    
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    #if there is some entry already there for someone, just update it instead of making a new entry
    if(len(rows) <= 0):
        query = ("INSERT INTO response_db (gameID, roundNum, questionID, sender, deviceType, delta, isCorrect) VALUES ("+str(gameID)+", "+str(roundNum)+", "+str(questionID)+", \'"+str(sender)+"\', \'"+str(deviceType)+"\', "+str(delta)+", "+str(isCorrect)+")")
    else:
        query = ("UPDATE response_db SET isCorrect="+str(isCorrect)+", delta="+str(delta)+",questionID="+str(questionID)+",roundNum="+str(roundNum)+" WHERE sender=\'"+str(sender)+"\'")
    cnx.query(query)
    cnx.commit()
    #create a mySQL query and commit to database relevant information for logging message
 
if method_type == 'POST':
    gameID = form.getvalue("gameID")
    roundNum = form.getvalue("roundNum")
    questionID = form.getvalue("questionID")
    sender = form.getvalue("sender")
    deviceType = form.getvalue("deviceType")
    delta = form.getvalue("delta")
    isCorrect = form.getvalue("isCorrect")
    #TODO: currentScore actually only gets incremented if this response has the smallest delta of all the items in DB with the same roundNum...
    #TODO: remove currentScore from posted info in teensytrivia.ino
    #TODO: modify methods in the .py files to not expect currentScore key/value pair
    currentScore = form.getvalue("currentScore")

    #sender = "cnordy"
    #delta = 0.2
    #isCorrect = 0
    #currentScore = 110
    #gameID = 0
    #deviceType = "teensey"
    #questionID = 0
    #roundNum = 1
    
    insertIntoDB(gameID,roundNum,questionID,str(sender),deviceType,delta,isCorrect)

elif method_type == 'GET':
    # Now pull data from database and compute on it
    query = ("SELECT * FROM response_db WHERE isCorrect=1 ORDER BY delta") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    cnx.commit()
    #print leaderboard stuff now
    currentRound= 0
    currentWinner = ""
    totalPlayers = len(rows)#temporary, add up all players who have answered
    if(form.getvalue('deviceType') == 'teensy' or form.getvalue('deviceType') == 'teensey'):
        for i in range(len(rows)):
            if i == 0:#the first one in this list has gone the fastest
                currentWinner = rows[i][4].decode("utf-8")
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])#set to the highest round value
        query = ("SELECT * FROM response_db WHERE isCorrect=0 ORDER BY delta") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        cnx.commit()
        totalPlayers = totalPlayers + len(rows)#add up all the rest of the players
        for i in range(len(rows)):
            if( i == 0 and len(currentWinner) == 0):#if no one got correct award fastest wrong one
                currentWinner = rows[i][4].decode("utf-8")
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])#set to the highest round value
        query = ("SELECT * FROM response_db WHERE roundNum=" + str(currentRound)) #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        cnx.commit()
        if(len(rows) == totalPlayers):
            query = ("SELECT * FROM response_db WHERE sender=\'"+str(currentWinner)+"\'")
            cnx.query(query)
            result = cnx.store_result()
            rows = result.fetch_row(maxrows=0,how=0) #what does this do?
            cnx.commit()
            newScore = int(rows[0][0])
            if(rows[0][0] is None):
                newScore = 0
            newScore = newScore + 10 #score points for winning
            query = ("UPDATE response_db SET currentScore="+str(newScore)+" WHERE sender=\'"+str(currentWinner)+"\'")
            cnx.query(query)
            cnx.commit()
        print("<w>" + currentWinner + "</w>")

    else:
        print('<h1>Current Question Results</h1>')
        print("<h2> These have answered correctly:</h2><p></p>")
  
        for i in range(len(rows)):
            if i == 0:#the first one in this list has gone the fastest
                currentWinner = rows[i][4].decode("utf-8")
                print(currentWinner)
            print("<p>" + str(i+1) + ".")
            #unpack('h', rows[i][7])[0]) decodes bit
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])#set to the highest round value
            print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
            print("</p>")
        query = ("SELECT * FROM response_db WHERE isCorrect=0 ORDER BY delta") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        cnx.commit()
        totalPlayers = totalPlayers + len(rows)#add up all the rest of the players
        print("<h2> These have answered wrong:</h2><p></p>")
        for i in range(len(rows)):
            if( i == 0 and len(currentWinner) == 0):#if no one got correct award fastest wrong one
                currentWinner = rows[i][4].decode("utf-8")
                print(currentWinner)
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])#set to the highest round value
            print("<p>" + str(i+1) + ".")
            print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
            print("</p>")
        query = ("SELECT * FROM response_db WHERE roundNum=" + str(currentRound)) #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        cnx.commit()
        print("<p>Round Status: " + str(len(rows)) + "/" + str(totalPlayers) + " have answered Round #" + str(currentRound))
        if(len(rows) == totalPlayers):
            print("<R>Y</R><W>" + currentWinner + "</W>")#finished turn if this is true
            query = ("SELECT currentScore FROM response_db WHERE sender=\'"+str(currentWinner)+"\'")
            cnx.query(query)
            result = cnx.store_result()
            rows = result.fetch_row(maxrows=0,how=0) #what does this do?
            cnx.commit()
            #print(rows)
            if(len(rows) > 0 and rows[0][0] is None):
                newScore = 0
            elif(len(rows) > 0):
                newScore = int(rows[0][0])
            if(len(rows) > 0):
                #print("NEW SCORE : " + str(newScore))
                newScore = newScore + 10 #score points for winning
                #print("NEW SCORE : " + str(newScore))
                query = ("UPDATE response_db SET currentScore="+str(newScore)+" WHERE sender=\'"+str(currentWinner)+"\'")
                cnx.query(query)
                cnx.commit()
        else:
            print("<R>N</R>")
        print("</p><p>Total Players : " + str(totalPlayers) + "</p>")

#TODO: make sure this printing works in 2 ways: one for web, one for teensy

cnx.close()#close so we don't go over max connections
print('</html>')
