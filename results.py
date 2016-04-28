import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())



# Do some HTML formatting
print( "Content-type:text/html\r\n\r\n")
print('<html>')
print('<head>')
print('<title>Posts Trivia Responses to Database</title>')
print('</head>')
print('<body>')

#Set up the server connection
cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')


method_type = get_method_type()

form  = get_params()

if method_type == "GET": #leaderboard
    # Now pull data from database and compute on it
    shouldDelete = False
    if 'shouldDelete' in form.keys():
        shouldDelete = form.getvalue("shouldDelete")[0] == "T"
    if(shouldDelete):
        query = ("DELETE FROM response_db")
        try:
            cnx.query(query)
            cnx.commit()
        except:
           # Rollback in case there is any error
           cnx.rollback()
        query = ("SELECT * FROM response_db ORDER BY currentScore") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        print(rows)
        
    else:
        query = ("SELECT * FROM response_db ORDER BY currentScore") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        
        #print leaderboard stuff now
        print('<h1>LEADERBOARD</h1>')
        print('<h2>Your current leaders are:</h2>')
        playersCount = 0
        for i in range(len(rows)):
            if rows[i][8] != None:
                playersCount = playersCount + 1
                print('<p>'+ str(playersCount) + "." + rows[i][4].decode("utf-8") + " : " + str(rows[i][8]))
                print('</p>')
            
        #later: format the results of the query to show the top scorers and their scores
        #also later: make sure this printing works in 2 ways: one for web, one for teensy
        

########## DO NOT CHANGE ANYTHING BELOW THIS LINE ##########
print('</body>')
print('</html>')




