const mongoose = require('mongoose');
const {Schema} = mongoose;

const gyroSchema = new Schema({
    time: {type: String},
    gyroX: { type: Number },
    gyroY: { type: Number },
    gyroZ: { type: Number },
    gyroX_hat: { type: Number },
    gyroY_hat: { type: Number },
    gyroZ_hat: { type: Number },
    accelX: { type: Number },
    accelY: { type: Number },
    accelZ: {type: Number},
    accelX_hat: { type: Number },
    accelY_hat: { type: Number },
    accelZ_hat: {type: Number},
});

module.exports = mongoose.model('gyro', gyroSchema);