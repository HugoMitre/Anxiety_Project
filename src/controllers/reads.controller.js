const reads = require('../models/reads');
const readsCtrl = {};

readsCtrl.getData = async(req, res) => {
    const data = await reads.find();
    // await reads.collection.drop();
    res.json(data);
}

readsCtrl.saveData = async(req, res) => {

    console.log(req.body);
    
    const lectura = new reads(req.body);
    // lectura.red = req.body.red;
    // lectura.ir = req.body.ir;
    // lectura.hr = req.body.hr;
    // lectura.validHR = req.body.validHR;
    // lectura.spo2 = req.body.spo2;
    // lectura.validSpo2 = req.body.validSpo2;

    await lectura.save();

    res.json({
        status: 201,
        msg: "read saved"
    });
}

readsCtrl.prueba = () => {
    
}

module.exports = readsCtrl; 