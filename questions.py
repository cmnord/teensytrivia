#6.S08 FINAL PROJECT
#questions.py
#GET returns the current question and makes sure this is a unique question for the
#specific question for all players in the database. Reads in questions from
#external JSON file

import urllib.request
import json
from random import randint, shuffle
import cgi
import _mysql

exec(open("/var/www/html/student_code/LIBS/s08libs.py").read())

# Do some HTML formatting at the very top!
print( "Content-type:text/html\r\n\r\n")
print('<html>')

p = 0 #the index of the random question
cnx = _mysql.connect(user='cnord_jennycxu', passwd='Pg8rdAyj',db='cnord_jennycxu')

method_type = get_method_type()
form = cgi.FieldStorage() #get specified parameters!

#open the external JSON with the questions for reading
r = urllib.request.urlopen('https://api.myjson.com/bins/46ciq')
html = r.read().decode('utf8') 
json_data = json.loads(html)

#update the database entry of every player to flag that a new question has been generated for the round
def updateQuestionID(questionID):
    query = ("SELECT sender FROM response_db")
    cnx.query(query)
        
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    #flag values in every person's database entry to tell them that a unique question has been added
    for i in range(len(rows)):
        sender = (rows[i][0]).decode("utf-8")
        query = ("UPDATE response_db SET isCorrect=0,delta=-1,questionID="+str(questionID) +" WHERE sender=\'"+str(sender)+"\'")
        cnx.query(query)
        cnx.commit()
    cnx.close()
#Figure out who sent the initial question request, and generate unique question and return it
if method_type == 'GET':
    if('sender' in form.keys()):
        sender = form.getvalue('sender')
        query = ("SELECT * FROM response_db WHERE sender=\'"+ str(sender)+"\'")
        cnx.query(query)
        
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0)
        #Figure out if there is a question already generated and get that index, if not create a new one
        if(len(rows) > 0 and float(rows[0][6]) != -1):
            p = randint(0,(len(json_data["questions"]) - 1)) #random question index number
            updateQuestionID(p) 
        elif len(rows) > 0:
            p = int(rows[0][3])#set to the value in the db that is the questionID
        else:
            p = randint(0,(len(json_data["questions"]) - 1)) #random question index number
            
        #parse the json for the specified question index
        q_id = (json_data["questions"][p]["id"]).strip()
        prompt = json_data["questions"][p]["prompt"]

        #parse and get the list of answers
        ans_list = json_data["questions"][p]["ans_list"]
        ans_list = ans_list[1:-1] #cuts off [ and ]
        ans_list = ans_list.split(", ") #turns this into a real python list
        correct_ans = json_data["questions"][p]["correct_ans"]

        #print out the question in a nice way
        print("<h1>" + q_id + ". " + prompt + "</h1>")
        print("<ul>")
        alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        for i in range(len(ans_list)):
            print("<li><%s>%s</%s></li>" %(alph[i],ans_list[i],alph[i]))
        print("</ul><h4>" + correct_ans + "</h4>")
       

# Close HTML page
print('</html>')
