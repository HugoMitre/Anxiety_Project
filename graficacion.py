from collections import deque
import pymongo
import matplotlib.pyplot as plt
import copy

# Conectarse a la base de datos local de MongoDB
client = pymongo.MongoClient('mongodb://localhost:27017')
db = client['sensorData'] # Nombre de la base de datos
collection = db['reads'] # Nombre de la colección

data = deque(maxlen = 100)

previous_query = list()

while(True):
    current_query = list(collection.find().sort('time', pymongo.ASCENDING))

    if len(current_query) != len(previous_query):
        query1_ids = set(result['_id'] for result in current_query)
        query2_ids = set(result['_id'] for result in previous_query)

        new_ids = query1_ids - query2_ids
        new_documents = [doc for doc in current_query if doc['_id'] in new_ids]

        for i in new_documents:
            data.append(i['ir'])
            plt.clf()
            plt.plot(data)
            plt.pause(0.01)

    previous_query = current_query.copy()
    plt.pause(1)

    if not plt.get_fignums():
        break

client.close()  # Cerrar la conexión a la base de datos