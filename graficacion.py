from collections import deque
import pymongo
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft

# Conectarse a la base de datos local de MongoDB
client = pymongo.MongoClient('mongodb://localhost:27017') 
db = client['sensorData'] # Nombre de la base de datos
collection = db['reads'] # Nombre de la colección

data = deque(maxlen = 100)

previous_query = list()
window_fft = 15
while(True):
    current_query = list(collection.find().sort('time', pymongo.ASCENDING))

    if len(current_query) != len(previous_query):
        query1_ids = set(result['_id'] for result in current_query)
        query2_ids = set(result['_id'] for result in previous_query)

        new_ids = query1_ids - query2_ids
        new_documents = [doc for doc in current_query if doc['_id'] in new_ids]

        for i in new_documents:
            data.append(i['ir'])
            plt.figure(1)
            plt.clf()
            plt.plot(data)
            plt.pause(0.01)
            if len(data) == 100:
                if window_fft < 15:
                    window_fft += 1
                else:
                    window_fft = 0

                    ## PROCESAMIENTO PARA FOURIER
                    # Filtro Savitzky–Golay
                    yhat_3 = signal.savgol_filter(data, 7, 3)
                    yhat_0 = signal.savgol_filter(data, 31, 0)

                    y_prime = yhat_3 - yhat_0
                    y_optimus = y_prime - signal.savgol_filter(y_prime, 7, 0)

                    fourier = fft(y_optimus)

                    plt.figure(2)
                    plt.clf()
                    plt.plot(fourier)
                    plt.pause(0.01)


    previous_query = current_query.copy()
    plt.pause(1)

    if not plt.get_fignums():
        break

client.close()  # Cerrar la conexión a la base de datos