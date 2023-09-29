

from datetime import datetime
from dateutil import tz


time = datetime.now().astimezone(tz.gettz('Europe/Paris'))
text = time.strftime("%Y-%m-%d %H:%M:%S")

print(text)

exit(0)

from zoneinfo import ZoneInfo
#from pytz import timezone

t_local = datetime.now(tz=ZoneInfo('localtime'))
print(t_local)

t_paris = datetime.now(tz=ZoneInfo('Europe/Paris'))
print(t_paris)

text = t_local.strftime("%Y-%m-%d %H:%M:%S")
print(text)

text = t_paris.strftime("%Y-%m-%d %H:%M:%S")
print(text)
    
