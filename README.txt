Damien Lévin <dml_aon@hotmail.com>   
########### LastMoid 0.5 ###########
Lastmoid is a plasmoid for LastFm users. It displays your weekly ; 3 months ; 12 month 
or your overall charts (albums/tracks/artists) easily on your favorite desktop ;)
Since 0.5 it also displays recent played tracks.
If you have any suggestion, request, bug please let me know :).


Links :
http://www.last.fm/home
http://en.wikipedia.org/wiki/Last.fm


########### Changelog ###########
0.5

    - Cut too long data
    - Recent played tacks available

0.4

    - Refresh bug fixed
    - 3 months / 12 months / Overall charts available
    - Album charts available
    - Add an Icon


0.3

    - Bug fixes
    - New logo
    - Adding a tool to set :
      * LastFm user
      * Chart


0.1 
    - First draft of the weekly artist chart



########### Install ###########

tar xvzf lastmoidXX.tar.gz
cd Lastmoid
cmake -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` .
make
make install (as root or with sudo)

