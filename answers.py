import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())
print( "Content-type:text/html\r\n\r\n")
print('<html>')

cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')

method_type = get_method_type()
form = cgi.FieldStorage() #get specified parameters!

if method_type == 'POST':
    sender = "asdf"
    deviceType = "asdf"
    gameID = 0
    questionID = 1
    roundNum = 0
    delta = 0.2
    isCorrect = False
    currentScore = 0
    gameID = form.getvalue("gameID")
    roundNum = form.getvalue("roundNum")
    questionID = form.getvalue("questionID")
    sender = form.getvalue("sender")
    deviceType = form.getvalue("deviceType")
    delta = form.getvalue("delta")
    isCorrect = form.getvalue("isCorrect")
    currentScore = form.getvalue("currentScore")
    
    
    cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')

    #create a mySQL query and commit to database relevant information for logging message
    query = ("INSERT INTO response_db (gameID, roundNum, questionID, sender, deviceType, delta, isCorrect, currentScore) VALUES (" + str(gameID) + ", " + str(roundNum)  + ", " + str(questionID) + ", \'" + str(sender)  + "\', \'" + str(deviceType)  + "\', " + str(delta)  + ", " + str(isCorrect) + ", " + str(currentScore) + ")")
    cnx.query(query)
    cnx.commit()
    #print("<br>Inserted into database success!")
    cnx.close()#close so we don't go over max connections
elif method_type == 'GET':
   # Now pull data from database and compute on it
    query = ("SELECT * FROM response_db") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    
    #print leaderboard stuff now
    print('<h1>What is in the Database</h1>')
    print(rows)
    #later: format the results of the query to show the top scorers and their scores
    #also later: make sure this printing works in 2 ways: one for web, one for teensy
print('</html>')
