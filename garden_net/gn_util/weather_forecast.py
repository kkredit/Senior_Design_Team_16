from forecastiopy import *

apikey = 'f119a2dd4a7d21069aebce2924af6f9a'

my_house = [42.92738, -85.62081]

fio = ForecastIO.ForecastIO(apikey, latitude=my_house[0], longitude=my_house[1])

if fio.has_daily() is True:
    daily = FIODaily.FIODaily(fio)
    print('Daily')
    print('Summary:', daily.summary)
    print('Icon:', daily.icon)
    print()
    for day in range(0, daily.days()):
        print('Day', day+1)
        for item in daily.get_day(day).keys():
            print(item + ' : ' + str(daily.get_day(day)[item]))
        print
        # Or access attributes directly for a given minute.
        # daily.day_7_time would also work
        print(daily.day_5_time)
        print()
    print()
else:
    print('No Daily data')