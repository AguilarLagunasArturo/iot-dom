import boto3
import json
import time
from datetime import datetime, timedelta

aws_dynamodb = boto3.client('dynamodb')

ACTIONS = [
    'get-device-state',
    'set-device-state',
    'toggle-device-state',
    'set-device-connection',
    'is-available',
    'setup-device'
]

def respond(res):
    return {
        'statusCode': 200,
        'body': json.dumps(res),
        'headers':{
            'content-type' : 'application/json',
        },
    }

def get_device_data(device_id):
    response = aws_dynamodb.get_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': device_id}})
    return response

def set_device_state(device_id, device_value):
    response = aws_dynamodb.update_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': device_id}},
        AttributeUpdates={
            'powerStateValue': {'Action': 'PUT', 'Value': {'S': device_value}},
            'lastConnection': {'Action': 'PUT', 'Value': {'S': get_utc_timestamp()}}
        }
    )
    return response

def setup_device(device_id):
    device_data = get_device_data(device_id)
    if device_data.get('Item'):
        response = aws_dynamodb.update_item(
            TableName='IotSkill',
            Key={'ItemId': {'S': device_id}},
            AttributeUpdates={
                'powerStateValue': {'Action': 'PUT', 'Value': {'S': 'OFF'}},
                'alexaState': {'Action': 'PUT', 'Value': {'S': device_data.get('Item').get('alexaState').get('S')}},
                'lastConnection': {'Action': 'PUT', 'Value': {'S': get_utc_timestamp()}}
            }
        )
        return response
    else:
        response = aws_dynamodb.update_item(
            TableName='IotSkill',
            Key={'ItemId': {'S': device_id}},
            AttributeUpdates={
                'powerStateValue': {'Action': 'PUT', 'Value': {'S': 'OFF'}},
                'alexaState': {'Action': 'PUT', 'Value': {'S': 'OFF'}},
                'lastConnection': {'Action': 'PUT', 'Value': {'S': get_utc_timestamp()}}
            }
        )
        return response

def set_device_connection(device_id):
    response = aws_dynamodb.update_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': device_id}},
        AttributeUpdates={
            'lastConnection': {'Action': 'PUT', 'Value': {'S': get_utc_timestamp()}}
        }
    )
    return response

def get_utc_timestamp(seconds=None):
    return time.strftime('%Y-%m-%dT%H:%M:%S.00Z', time.gmtime(seconds))

def is_time_exceeded(ts, dt=10):
    t_current = time.strptime(get_utc_timestamp(), '%Y-%m-%dT%H:%M:%S.00Z')
    t_old = time.strptime(ts, '%Y-%m-%dT%H:%M:%S.00Z')

    td1 = datetime.fromtimestamp(time.mktime(t_current)) - datetime.fromtimestamp(time.mktime(t_old))
    td2 = timedelta(seconds=dt)

    print('[td1]', td1)
    print('[td2]', td1)
    print('[td1 > td2]', td1 > td2)

    return td1 > td2

def lambda_handler(event, context):
    device_data = event['body']

    if not device_data:
        return respond('Empty body')

    device_data = json.loads(device_data)
    action = device_data['action']

    print('[MODE] {}'.format(action))

    if action == ACTIONS[0]:
        device_id = device_data['device-id']
        response = get_device_data(device_id)
        state = response['Item']['powerStateValue']['S']
        return respond('[{}] {}'.format(action, state))
    elif action == ACTIONS[1]:
        device_id = device_data['device-id']
        device_value = device_data['value']
        response = set_device_state(device_id, device_value)
        return respond('[{}] {}'.format(action, device_value))
    elif action == ACTIONS[2]:
        device_id = device_data['device-id']
        response = get_device_data(device_id)
        state = response['Item']['powerStateValue']['S']
        device_value = 'OFF' if state == 'ON' else 'ON'
        response = set_device_state(device_id, device_value)
        return respond('[{}] {}'.format(action, device_value))
    elif action == ACTIONS[3]:
        device_id = device_data['device-id']
        response = set_device_connection(device_id)
        response = get_device_data(device_id)
        value = response['Item']['lastConnection']['S']
        return respond('[{}] {}'.format(action, value))
    elif action == ACTIONS[4]:
        device_id = device_data['device-id']
        response = get_device_data(device_id)
        state = response['Item']['powerStateValue']['S']
        value = response['Item']['lastConnection']['S']
        return respond('[{}] {} - {}'.format(action, state, not is_time_exceeded(value)))
    elif action == ACTIONS[5]:
        device_id = device_data['device-id']
        setup_device(device_id)
        return respond('[{}] OFF'.format(action))
    else:
        return respond( '[ACTION] {}'.format( action ) )
