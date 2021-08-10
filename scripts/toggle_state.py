import requests
import json
import os

url = os.environ.get('IOT_SKILL_API')
device_uuid = 'iot-skill-00'

get_data = {
	'action' : 'get-device-state',
	'device-id' : device_uuid
}

set_data = {
	'action' : 'set-device-state',
	'device-id': device_uuid,
	'value' : '--'
}

toggle_data = {
	'action' : 'toggle-device-state',
	'device-id': device_uuid
}

def set_state(state='OFF'):
	global set_data
	set_data['value'] = state
	return requests.post(url, json=set_data)

def get_state():
	global get_data
	return requests.post(url, json=get_data)

def toggle_state():
	global toggle_data
	return requests.post(url, json=toggle_data)

r = toggle_state()

print(r.status_code)
print(r.url)
print(r.json())
