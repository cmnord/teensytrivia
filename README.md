#Teensy Trivia

6.S08 final project
Claire Nord and Jenny Xu

##To do
- Get SD card working: display images
- Replace buttons with joystick
- Test responses.py as interface between teensy and response_db
- Figure out how rounds are going to look in the database: 
- 1. Delete all entries in the database after each round and check for when there are entries = # players
- 2. Push one entry for each player and update each person's row until you get everyone's roundID = currentRound'
-Plus points for fastest + set points for correct answer
-Figure out how to get each player in + insert initial database entry
-Test advancing round = when all players have an answer in database 
-Display leaderboard on teensey screen then a couple seconds later shows new quesion
-End game support
-in answers.py when len(rows) == num players can calclate winner take the top entry in database as winner and add to their currentscore, and then teenseytrivia.ino GET's currentScore and updates local score and then all teensey's pull new question'

##Milestones
- April 28: Multiplayer support: how to sync up, join a game together
- May 1: Dynamic leaderboard
- May 8: Menu on Teensy: join new game, view leaderboard
- May 15: Graphics and sound