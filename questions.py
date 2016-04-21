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
correct_ans = json_data["questions"][p]["correct_ans"]
print("<b>" + q_id + ". " + prompt + "</b>")
print("<ul><li>" + ans_list + "</li>")
print("<li>" + correct_ans + "</li></ul>")


########## DO NOT CHANGE ANYTHING BELOW THIS LINE ##########
# Close HTML page
print('</html>')
