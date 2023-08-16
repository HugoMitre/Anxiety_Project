const reads = require('../models/reads');
const phase = require('../models/phase');
const phaseCtrl = {};

phaseCtrl.saveSensorData = async(req, res) => {
    const data = await reads.find();
    // await reads.collection.drop();

    console.log(req.body);

    const fase = new phase();
    fase.subject = req.body.subject;
    fase.phase = req.body.phase;
    fase.data_reads = data;
    fase.date = new Date();

    await fase.save();

    res.json({
        status: 201,
        msg: "phase saved"
    });
}

module.exports = phaseCtrl;