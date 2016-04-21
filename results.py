import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())



# Do some HTML formatting
print( "Content-type:text/html\r\n\r\n")
print('<html>')
print('<head>')
print('<title>Posts Trivia Responses to Database</title>')
print('<body>')

#Set up the server connection
cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')


method_type = get_method_type()

form  = get_params()
print("form= ")
print(form)

if method_type == "GET": #leaderboard
    # Now pull data from database and compute on it
    query = ("SELECT sender FROM response_db ORDER BY currentScore DESC LIMIT 5") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
    cnx.query(query)
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0) #what does this do?
    
    #print leaderboard stuff now
    print('<h1>LEADERBOARD</h1>')
    print(rows)
    #later: format the results of the query to show the top scorers and their scores
    #also later: make sure this printing works in 2 ways: one for web, one for teensy


elif method_type == "POST":
    if "gameID" in form.keys() and "roundNum" in form.keys() and "questionID" in form.keys() and "sender" in form.keys() and "deviceType" in form.keys() and "delta" in form.keys():
        print("Pushing to Database")
        gameID = form.getvalue("gameID")
        roundNum = form.getvalue("roundNum")
        questionID = form.getvalue("questionID")
        sender = form.getvalue("sender")
        deviceType = form.getvalue("deviceType")
        delta = form.getvalue("delta")

        query = ("INSERT INTO response_db (gameID, roundNum, questionID, sender, deviceType, delta) VALUES (\'" + gameID + "\', \'" + roundNum  + "\', \'" + questionID + "\', \'" + sender  + "\', \'" + deviceType  + "\', \'" + delta  + "\')")
        cnx.query(query)
        cnx.commit()
        print("<br>Inserted into database success!")
    else:
        print("<br>Not all post variables were received...")

########## DO NOT CHANGE ANYTHING BELOW THIS LINE ##########
print('</body>')
print('</html>')




