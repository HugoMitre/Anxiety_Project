const mongoose = require('mongoose');
const {Schema} = mongoose;

const phaseSchema = new Schema({
    subject: { type: String, required: true },
    phase: { type: String, required: true },
    data_reads: { type: Array, required: true },
    date: { type: Date }
});

module.exports = mongoose.model('phase', phaseSchema);