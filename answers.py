#6.S08 FINAL PROJECT
#answers.py
#GET returns who has currently answered and if they are wrong or not. Also calculates
#when each round is over and returns the winner of each round
#POST posts to the database with the passed in values

import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())
print( "Content-type:text/html\r\n\r\n")
print('<html>')

#make connection to database
cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')
method_type = get_method_type()
form = cgi.FieldStorage() #get specified parameters!

#method that inserts the current state of the sender into the database
def insertIntoDB(gameID,roundNum,questionID,sender,deviceType,delta,isCorrect):
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
#method that gives the winner of each round points for winning that round
def updateScore(currentWinner):
    #only select the winner's entry from the data when it's not round 0 and they've actually won the game
    query = ("SELECT currentScore FROM response_db WHERE sender=\'"+str(currentWinner)+"\' AND delta != 0 AND roundNum != 0")
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    cnx.commit()
    #Grab the score of the sender from the database 
    if(len(rows) > 0 and rows[0][0] is None):
        newScore = 0
    elif(len(rows) > 0):
        newScore = int(rows[0][0])
    #if the winner exists, the add 10 points to their score and update it in the database
    if(len(rows) > 0):
        newScore = newScore + 10 
        query = ("UPDATE response_db SET currentScore="+str(newScore)+" WHERE sender=\'"+str(currentWinner)+"\'")
        cnx.query(query)
        cnx.commit()
        flagForNextRound()
#method that flags selects the rest of the players and flag their teensey's for the next round using delta=0
def flagForNextRound():
    query = ("SELECT sender FROM response_db")
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    for i in range(len(rows)):
        sender = (rows[i][0]).decode("utf-8")
        query = ("UPDATE response_db SET delta=0 WHERE sender=\'"+str(sender)+"\'")
        cnx.query(query)
        cnx.commit()
#find who is currently winning in the specified category
def findCurrentRoundStatus(rows):
    for i in range(len(rows)):
        if i == 0:
            currentWinner = rows[i][4].decode("utf-8")
        if(int(rows[i][2]) > currentRound):
            currentRound = int(rows[i][2])
    return currentWinner,currentRound
#if posting something to the database, then send the corresponding values
if method_type == 'POST':
    gameID = form.getvalue("gameID")
    roundNum = form.getvalue("roundNum")
    questionID = form.getvalue("questionID")
    sender = form.getvalue("sender")
    deviceType = form.getvalue("deviceType")
    delta = form.getvalue("delta")
    isCorrect = form.getvalue("isCorrect")
    currentScore = form.getvalue("currentScore")
    
    insertIntoDB(gameID,roundNum,questionID,str(sender),deviceType,delta,isCorrect)
#if getting from this file then show the current entries into the database ordered by if correct
elif method_type == 'GET':
    #select the players who got it correct and show the winners highest up
    query = ("SELECT * FROM response_db WHERE isCorrect=1 ORDER BY delta ASC") 
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    cnx.commit()
    currentRound= 0
    currentWinner = ""
    #calculate the number of players in the game
    totalPlayers = len(rows)

    #if on teensey, format it differently
    if(form.getvalue('deviceType') == 'teensy' or form.getvalue('deviceType') == 'teensey'):
        #the first one in this list has gone the fastest, and figure out who is winning and what round they're on
        currentWinner,currentRound = findCurrentRoundStatus(rows)

        #calculate those who have gotten it wrong and order by who did it the fastest
        query = ("SELECT * FROM response_db WHERE isCorrect=0 ORDER BY delta ASC")
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0)
        cnx.commit()
        totalPlayers = totalPlayers + len(rows)#add up all the rest of the players
        #if no one got correct award fastest wrong one
        currentWinner,currentRound = findCurrentRoundStatus(rows)

        #show the current entries in this round
        query = ("SELECT * FROM response_db WHERE roundNum=" + str(currentRound))
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        cnx.commit()

        #if everyone has answered, show the winner
        if(len(rows) == totalPlayers):
            updateScore(currentWinner);
        print("<w>" + currentWinner + "</w>")

    else:
        #if on web, do the same thing as teensey but print out more lines for easy readability
        print('<h1>Current Question Results</h1>')
        print("<h2> These have answered correctly:</h2><p></p>")
        #show who got it right
        for i in range(len(rows)):
            if i == 0:
                currentWinner = rows[i][4].decode("utf-8")
                print(currentWinner)
            print("<p>" + str(i+1) + ".")
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])
            print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
            print("</p>")
        #show who got it wrong
        query = ("SELECT * FROM response_db WHERE isCorrect=0 ORDER BY delta ASC")
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) 
        cnx.commit()
        totalPlayers = totalPlayers + len(rows)
        print("<h2> These have answered wrong:</h2><p></p>")
        for i in range(len(rows)):
            if( i == 0 and len(currentWinner) == 0):
                currentWinner = rows[i][4].decode("utf-8")
                print(currentWinner)
            if(int(rows[i][2]) > currentRound):
                currentRound = int(rows[i][2])
            print("<p>" + str(i+1) + ".")
            print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
            print("</p>")
        #Figure out if the round is over by checking how many people have answered this round
        query = ("SELECT * FROM response_db WHERE roundNum=" + str(currentRound))
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) 
        cnx.commit()
        #show how many players have currently answered out of total players
        print("<p>Round Status: " + str(len(rows)) + "/" + str(totalPlayers) + " have answered Round #" + str(currentRound))
        #finished turn if this all players have answered the current Question and print the winner
        if(len(rows) == totalPlayers):
            print("<R>Y</R><W>" + currentWinner + "</W>")
            updateScore(currentWinner);
        else:
            print("<R>N</R>")
        print("</p><p>Total Players : " + str(totalPlayers) + "</p>")
cnx.close()#close so we don't go over max connections
print('</html>')

