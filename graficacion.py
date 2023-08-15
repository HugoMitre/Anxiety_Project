from collections import deque
import pymongo
import matplotlib.pyplot as plt
import numpy as np

# Conectarse a la base de datos local de MongoDB
client = pymongo.MongoClient('mongodb://localhost:27017')
db = client['sensorData'] # Nombre de la base de datos
collection = db['reads'] # Nombre de la colección

datos = deque(maxlen = 100)

for i in collection.find().sort('time', pymongo.ASCENDING):
    datos.append(i['ir'])
    print(datos)
    plt.clf()
    plt.plot(datos)
    plt.pause(0.1)

client.close()  # Cerrar la conexión a la base de datos