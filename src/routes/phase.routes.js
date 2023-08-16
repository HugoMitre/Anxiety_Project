// Rutas que se crearán para los endpoints que correspondan a los datos del sensor del arduino.
const express = require('express');
const router = express.Router();
const phaseCtrl = require('../controllers/phase.controller');

router.get('/',(req, res) => { // Formulario para mandar agrupar datos 
    res.render('index');
});
router.post('/save', phaseCtrl.saveSensorData);

module.exports = router;