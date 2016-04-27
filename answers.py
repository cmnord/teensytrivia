import _mysql
import cgi
from struct import *
exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())
print( "Content-type:text/html\r\n\r\n")
print('<html>')

cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')

method_type = get_method_type()
form = cgi.FieldStorage() #get specified parameters!
def insertIntoDB(gameID,roundNum,questionID,sender,deviceType,delta,isCorrect,currentScore):
    cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')
    query = ("SELECT * FROM response_db WHERE sender=\'"+ str(sender)+"\'")
    cnx.query(query)
    
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    #if there is some entry already there for someone, just update it instead of making a new entry
    if(len(rows) <= 0):
        query = ("INSERT INTO response_db (gameID, roundNum, questionID, sender, deviceType, delta, isCorrect, currentScore) VALUES (" + str(gameID) + ", " + str(roundNum)  + ", " + str(questionID) + ", \'" + str(sender)  + "\', \'" + str(deviceType)  + "\', " + str(delta)  + ", " + str(isCorrect) + ", " + str(currentScore) + ")")
    else:
        query = ("UPDATE response_db SET currentScore=" + str(currentScore) + ", isCorrect=" + str(isCorrect) + ", delta=" + str(delta) + ",questionID=" + str(questionID) + ",roundNum=" + str(roundNum) + " WHERE sender=\'" + str(sender) +"\'")
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
    currentScore = form.getvalue("currentScore")

    #sender = "cnordy"
 #  delta = 0.2
 #   isCorrect = 0
#    currentScore = 110
 #   gameID = 0
#    deviceType = "teensey"
#    questionID = 0
#    roundNum = 1
    
    insertIntoDB(gameID,roundNum,questionID,sender,deviceType,delta,isCorrect,currentScore)
elif method_type == 'GET':
    # Now pull data from database and compute on it
    print('<h1>Current Question Results</h1>')
    
    query = ("SELECT * FROM response_db WHERE isCorrect=1 ORDER BY delta") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    cnx.commit()
    #print leaderboard stuff now
    print("<h2> These have answered correctly:</h2><p></p>")
    for i in range(len(rows)):
        print("<p>" + str(i+1) + ".")
        #unpack('h', rows[i][7])[0]) decodes bit
        print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
        print("</p>")
    query = ("SELECT * FROM response_db WHERE isCorrect=0 ORDER BY delta") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    cnx.commit()
    print("<h2> These have answered wrong:</h2><p></p>")
    for i in range(len(rows)):
        print("<p>" + str(i+1) + ".")
        print(rows[i][4].decode("utf-8") + " answered question " + str(rows[i][2]) + " and their response time was " + str(rows[i][6]) + " and their current score is " + str(rows[i][8]))
        print("</p>")
    cnx.close()#close so we don't go over max connections
    #later: format the results of the query to show the top scorers and their scores
    #also later: make sure this printing works in 2 ways: one for web, one for teensy

print('</html>')
