"""
	Date: 04/29/2016
	Author: Charles A. Kingston

	send_email.py

	This script is used to send the emails from the GardeNetServer@gmail.com
	account to the user who has signed up to receive the emails.
"""
# Third Party Imports
# Import smtplib for the actual sending function
import smtplib

class Alert:
	"""
		An explicit constructor that takes in the message and sends
		the email to the user that is sighed up

		@:param alert_message, str of the message that is to be sent
	"""
	def __init__(self, alert_message: str):
		TEXT = alert_message

		#https://www.google.com/settings/security/lesssecureapps
		f = open("user_info.txt")
		for line in f:
			current = line.split(" ")
			if str(current[0]).upper() == "EMAIL:":
				email_address = str(current[1].split('\n')[0])
		TO = email_address
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
		#for email in TO:
		try:
			s.sendmail(gmail_sender, [TO], BODY)
			#print("Email Sent!")
		except Exception as e:
			print("Error sending email")
			print(str(e))
		s.quit()

if __name__ == "__main__":
	Alert()