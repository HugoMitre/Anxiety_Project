const mongoose = require('mongoose');
const {Schema} = mongoose;

const readsSchema = new Schema({
    time: { type: String },
    red: { type: String },
    ir: { type: String },
    hr: { type: String },
    validHr: {type: String},
    spo2: { type: String },
    validSpo2: {type: String},
});

module.exports = mongoose.model('reads', readsSchema);