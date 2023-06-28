// Rutas que se crearán para los endpoints que correspondan al envío de datos del arduino a local.
const express = require('express');
const router = express.Router();
const readsCtrl = require('../controllers/reads.controller');

router.get('/', readsCtrl.getData);
router.post('/', readsCtrl.saveData);

module.exports = router;