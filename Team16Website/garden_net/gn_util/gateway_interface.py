from gateway_report import Garden
from send_email import Alert

class Report:
	def __init__(self, value: str):
		try:
			report = Garden(value)
			Alert(report)
		except:
			print("That's an incorrect string")

if __name__ == "__main__":
	it = Report("Hi")
	it2 = Report("01%0.99%0.99%0.99%{%1%0%0.99%209.3%1%[%1%60%]%[%2%30%]%[%3%75%]%}")