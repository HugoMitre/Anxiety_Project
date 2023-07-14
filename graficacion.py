import subprocess
import pymongo
import matplotlib.pyplot as plt

# Conectarse a la base de datos local de MongoDB
client = pymongo.MongoClient('mongodb://localhost:27017')
db = client['sensorData'] # Nombre de la base de datos
collection = db['reads'] # Nombre de la colección

# Configurar el gráfico
fig, ax = plt.subplots()

# Función para actualizar el gráfico en tiempo real
def update_plot():
    # Obtener los datos más recientes de la base de datos
    latest_data = collection.find().sort('time', pymongo.ASCENDING).limit(100)
    x = []
    # y = []
    for data in latest_data:
        x.append(data['red'])
        # y.append(data['timestamp'])

    # Actualizar el gráfico
    ax.clear()
    # ax.plot(x, y)
    ax.plot(x)
    plt.draw()

# Bucle principal para actualizar el gráfico continuamente
while True:
    update_plot()
    plt.pause(1)  # Pausa de 1 segundo entre actualizaciones

    # Comprobar si se ha cerrado la ventana de la gráfica
    if not plt.get_fignums():
        break

# Cerrar la conexión a la base de datos
client.close()