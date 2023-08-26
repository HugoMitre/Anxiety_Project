const mongoose = require('mongoose');
const {Schema} = mongoose;

const readsSchema = new Schema({
    time: { type: String },
    red: { type: Number },
    ir: { type: Number },
    hr: { type: Number },
    spo2: { type: Number },
    gyroX: {type: Number},
    gyroY: {type: Number},
    gyroZ: {type: Number}
});

module.exports = mongoose.model('reads', readsSchema);