#6.S08 FINAL PROJECT
#April 14th deadline: get working prototype of ìquestions.pyî and do database setup.
#Pick which tables the database will have and what columns those tables will contain.
#This will probably require deciding on your ìjoin gameî functionality.


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
r = urllib.request.urlopen('https://api.myjson.com/bins/46ciq')
html = r.read().decode('utf8') 
json_data = json.loads(html)
#someone chooses a question ID -> pushes into everyone's db entry
#question ID is used to get the same value everytime
#unshuffled, and it is shuffled on the user's teensey not on the server though
#when everyone gets the question ID in the database, they are set to delta=0 and delta 0
#is a flag that they haven't answered yet
#update quesitonID's for all players by iterating through the database
def updateQuestionID(questionID):
    query = ("SELECT sender FROM response_db")
    cnx.query(query)
        
    result = cnx.store_result()
    rows = result.fetch_row(maxrows=0,how=0)
    print(rows)
    for i in range(len(rows)):
        sender = (rows[i][0]).decode("utf-8")
        query = ("UPDATE response_db SET isCorrect=0,delta=-1,questionID="+str(questionID) +" WHERE sender=\'"+str(sender)+"\'")
        cnx.query(query)
        cnx.commit()
    cnx.close()
 
if method_type == 'GET':
    if('sender' in form.keys()):
        sender = form.getvalue('sender')
        query = ("SELECT * FROM response_db WHERE sender=\'"+ str(sender)+"\'")
        cnx.query(query)
        
        result = cnx.store_result()
        rows = result.fetch_row(maxrows=0,how=0)
        #if there is some entry already there for someone, just update it instead of making a new entry
        if(len(rows) > 0 and float(rows[0][6]) != -1):#when delta isn't 0 = flag for when there is a previous question there
           p = randint(0,(len(json_data["questions"]) - 1)) #random question index number
           updateQuestionID(p) #update which question for all players
           print("New question generated for Round is:" + str(p))
        elif len(rows) > 0:
            print(float(rows[0][6]));
            p = int(rows[0][3])#set to the value in the db that is the questionID
            print("Already have a question for Round:" + str(p))
        else:
            p = randint(0,(len(json_data["questions"]) - 1)) #random question index number
            
 
        q_id = (json_data["questions"][p]["id"]).strip()
        prompt = json_data["questions"][p]["prompt"]

        ans_list = json_data["questions"][p]["ans_list"]
        ans_list = ans_list[1:-1] #cuts off [ and ]
        ans_list = ans_list.split(", ") #turns this into a real python list
        #shuffle(ans_list) #order of answers is now shuffled

        correct_ans = json_data["questions"][p]["correct_ans"]
        print("<h1>" + q_id + ". " + prompt + "</h1>")
        print("<ul>")
        alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        for i in range(len(ans_list)):
            print("<li><%s>%s</%s></li>" %(alph[i],ans_list[i],alph[i]))
        print("</ul><h4>" + correct_ans + "</h4>")
       

# Close HTML page
print('</html>')
