#Teensy Trivia

6.S08 final project
Claire Nord and Jenny Xu

##To do
- Get SD card working: display images
- Replace buttons with joystick
- Figure out how rounds are going to look in the database: 
    1. Delete all entries in the database after each round and check for when there are entries = # players
    2. Push one entry for each player and update each person's row until you get everyone's roundID = currentRound'
    3. Plus points for fastest + set points for correct answer
- Figure out how to get each player in + insert initial database entry
- Test advancing round = when all players have an answer in database 
- Display leaderboard on teensey screen then a couple seconds later shows new quesion
- End game support
- to start a game, player starts by entering game by pressing A button: inserts entry into database with roundnumber 0 and also currentScore of 0
- when all players have entered, player initiates game by pressing B (can change so all players press B but for now only one person needs to out of everyone then when that happens that specific teensey sends POSt request to question.py to change the round number and questionID
- all players are sending continuous GETS to question.py anticipating the round num change and after the person sends initial request then questions.py now loads question in from json file and then roundnum++ and when GETTING they see that the new roundnum is more so it displays the quesiton and listens for input on each separately and sends the response to answers.py and changes screen to a "waiting for all responses" screen with current leaderboard showing while GETTING from answers.py instead so I know when enough players have submitted responses"
- when the last player submits answer to answer.py does a check of who won
- in answers.py when len(rows) == num players can calclate winner take the top entry in database as winner and print winner at the top and if the person GET's the current winner and it matches themself, add to their currentscore, and then teenseytrivia.ino GET's currentScore and updates local score then that winner's teensey sends a POST to questions and the flag shows the next question 
- can add leaderboard showing later or in between
-Add score correctly to game
-show the winner of each round
-need to clear database at the end of each round
-check why questionID is always 1 and also check that questions are always the same across teenseys(sender=jennycxu&questionID=1&deviceType=teensy&gameID=932&roundNum=0&delta=0.000&isCorrect=0
)

##Milestones
- April 28: Multiplayer support: how to sync up, join a game together
- May 1: Dynamic leaderboard
- May 8: Menu on Teensy: join new game, view leaderboard
- May 15: Graphics and sound
