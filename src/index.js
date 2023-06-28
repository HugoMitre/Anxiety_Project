//npm i express, mongooose, morgan, nodemon -D, dotenv, ejs, body-parser
const express = require('express');
const morgan = require('morgan');
const app = express();

const phaseRoutes = require("./routes/phase.routes");
const readsRoutes = require("./routes/reads.routes");
const {mongoose} = require('./database');

// Settings
const port = process.env.PORT || 3000;
app.set('views', 'src/views');
app.set('view engine', 'ejs');

// Middelware
app.use(express.json());    // Posibilidad de trabajar con archivos json
app.use(morgan('dev'));

// Routes
app.use('/api/phase',phaseRoutes);
app.use('/api/reads',readsRoutes);


app.listen(port, ()=> console.log('Server listening on port: ', port));