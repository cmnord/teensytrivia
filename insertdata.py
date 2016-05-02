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
 #close so we don't go over max connectionsz
    #create a mySQL query and commit to database relevant information for logging message
 
if method_type == 'GET':
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
    print('<h1>Current Question Results</h1>')
    query = ("SELECT * FROM response_db") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    cnx.commit()
    #print leaderboard stuff now
    print("<h2> These have answered correctly:</h2><p></p>")
    for i in range(len(rows)):
        print("<p>" + str(i+1) + ".")
        print(rows[i])
        print("</p>")
    cnx.close()
    #TODO: format the results of the query to show the top scorers and their scores
    #TODO: make sure this printing works in 2 ways: one for web, one for teensy

print('</html>')
