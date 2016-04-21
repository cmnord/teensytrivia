#6.S08 FINAL PROJECT
#April 14th deadline: get working prototype of ìquestions.pyî and do database setup.
#Pick which tables the database will have and what columns those tables will contain.
#This will probably require deciding on your ìjoin gameî functionality.


import urllib.request
import json
from random import randint


# Do some HTML formatting at the very top!
print( "Content-type:text/html\r\n\r\n")
print('<html>')

r = urllib.request.urlopen('https://api.myjson.com/bins/46ciq')
html = r.read().decode('utf8') 
#print (r.text)
json_data = json.loads(html)
p = randint(0,(len(json_data["questions"]))) - 1 #random question index number
q_id = (json_data["questions"][p]["id"]).strip()
prompt = json_data["questions"][p]["prompt"]

ans_list = json_data["questions"][p]["ans_list"]
ans_list = ans_list[1:-1] #cuts off [ and ]
ans_list = ans_list.split(", ") #turns this into a real python list

correct_ans = json_data["questions"][p]["correct_ans"]
print("<h1>" + q_id + ". " + prompt + "</h1>")
print("<ul>")
alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
for i in range(len(ans_list)):
    print("<li><%s>%s</%s></li>" %(alph[i],ans_list[i],alph[i]))
print("</ul><h4>" + correct_ans + "</h4>")


########## DO NOT CHANGE ANYTHING BELOW THIS LINE ##########
# Close HTML page
print('</html>')
