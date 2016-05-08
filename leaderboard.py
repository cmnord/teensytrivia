#6.S08 FINAL PROJECT
#Leaderboard.py
#GET returns the current leaderboard, formatted by teensey, also can delete and clear the current leaderboard

import _mysql
import cgi

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())


#HTML formatting
print( "Content-type:text/html\r\n\r\n")
print('<html>')

#Set up the server connection
cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')
method_type = get_method_type()
form  = get_params()

#Display leaderboard
if method_type == "GET":
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
        #display the leaderboard by ordering all players
        query = ("SELECT * FROM response_db ORDER BY currentScore") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        print(rows)
        
    else:
        #print leaderboard so that teensey can pull information to display
        query = ("SELECT * FROM response_db ORDER BY currentScore DESC") #should return senders with 5 highest scores (bug: there will be duplicates from the same game...)
        cnx.query(query)
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0) #what does this do?
        #If on teensey, display the players and the winners
        if(form.getvalue('deviceType') == 'teensy' or form.getvalue('deviceType') == 'teensey'):
            print('LEADERBOARD\n')
            playersCount = 0
            winnerScore = 0
            currentWinner = ''
            for i in range(len(rows)):
                if rows[i][8] != None:
                    playersCount = playersCount + 1
                    print(str(playersCount) + "." + rows[i][4].decode("utf-8") + " : " + str(rows[i][8]) + "\n")
                    if(int(rows[i][8]) >= winnerScore):
                        winnerScore = int(rows[i][8])
                        currentWinner = rows[i][4].decode("utf-8")
                else :
                    playersCount = playersCount + 1
                    print(str(playersCount) + "." + rows[i][4].decode("utf-8") + " : 0\n")
            print("<b>Current winner is <w>"+currentWinner+"</w> with a score of <s>"+str(winnerScore)+"</s>")
        else:
            #If on web, display the players and the winners with more headers and formatting
            print('<head>')
            print('<title>Posts Trivia Responses to Database</title>')
            print('</head>')
            print('<body>')
            #print leaderboard stuff now
            print('<h1>LEADERBOARD</h1>')
            print('<h2>Your current leaders are:</h2>')
            playersCount = 0
            for i in range(len(rows)):
                if rows[i][8] != None:
                    playersCount = playersCount + 1
                    print('<p>'+ str(playersCount) + "." + rows[i][4].decode("utf-8") + " : " + str(rows[i][8]))
                    print('</p>')
                else :
                    playersCount = playersCount + 1
                    print('<p>'+ str(playersCount) + "." + rows[i][4].decode("utf-8") + " :0")
                    print('</p>')
                    
            print('</body>') 
#this has to be included!
print('</html>')


