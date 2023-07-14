const mongoose = require('mongoose');
const {Schema} = mongoose;

const readsSchema = new Schema({
    time: { type: String },
    red: { type: Number },
    ir: { type: Number },
    hr: { type: Number },
    validHR: {type: Number},
    spo2: { type: Number },
    validSpo2: {type: Number}
});

module.exports = mongoose.model('reads', readsSchema);