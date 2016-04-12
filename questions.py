#6.S08 FINAL PROJECT
#April 14th deadline: get working prototype of ìquestions.pyî and do database setup.
#Pick which tables the database will have and what columns those tables will contain.
#This will probably require deciding on your ìjoin gameî functionality.

import urllib.request
import json
from random import randint

r = urllib.request.urlopen('https://api.myjson.com/bins/46ciq')
html = r.read().decode('utf8') 
#print (r.text)
json_data = json.loads(html)
numquestions = randint(0,(len(json_data["questions"]))) - 1
print("\nQuestion " + (json_data["questions"][numquestions]['id']).strip())#question 1
print(json_data["questions"][numquestions]['prompt'])
print("\nPossible answers:" + json_data["questions"][numquestions]['ans_list'])
print("\nCorrect Answer is:" + json_data["questions"][numquestions]['correct_ans'] + "\n")


