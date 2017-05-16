import requests
url = 'https://cp-parking-process-api.run.aws-usw02-pr.ice.predix.io/api/configuration/sensor/3D2F63B1-AA1E-4C5A-A4B2-698F2ADDC2BB/image'
files = {'file': open('test20.jpg', 'rb')}
req = requests.put(url, files=files)
print req
