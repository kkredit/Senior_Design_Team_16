#from gateway_report import Report

# Import smtplib for the actual sending function
import smtplib
import time

class Alert:
	def __init__(self, alert_message: str):
		TEXT = alert_message

		#https://www.google.com/settings/security/lesssecureapps
		TO = ["cak33@students.calvin.edu", "knk9@students.calvin.edu", "jrc32@students.calvin.edu", "xj22@students.calvin.edu"]
		SUBJECT = "GardeNet Alert!"


		# gmail Credentials
		gmail_sender = "GardeNetServer@gmail.com"
		gmail_password = "Caledonia"

		# Create connection to service
		s = smtplib.SMTP('smtp.gmail.com', 587)
		s.ehlo()
		s.starttls()
		s.ehlo()
		s.login(gmail_sender, gmail_password)

		BODY = '\r\n'.join([
			'To: %s' %TO,
			'From: %s' %gmail_sender,
			'Subject: %s' %SUBJECT,
			'',
			TEXT
			])
		for email in TO:
			try:
				s.sendmail(gmail_sender, [email], BODY)
				#print("Email Sent!")
				time.sleep(10)
			except Exception as e:
				print("Error sending email")
				print(str(e))
		s.quit()

if __name__ == "__main__":
	Alert()