<?php

/**
 * @file
 */

function uc_payfast_complete($cart_id = 0) {
  if (variable_get('uc_payfast_mode', '') == 'sandbox') {
    define('SANDBOX', true);
  }
  else define('SANDBOX', false);
 
  // Variable Initialization
  $pmtToken = isset( $_GET['pt'] ) ? $_GET['pt'] : null;
   
  if(!empty($pmtToken)) {
    // Variable Initialization
    $error = false;
    $authToken = '<YOUR_PDT_KEY>';
    $req = 'pt='. $pmtToken .'&at='. $authToken;
    $data = array();
    $host = SANDBOX ? 'sandbox.payfast.co.za' : 'www.payfast.co.za';
 
    // Connect to server
    if(!$error) {
      // Construct Header
      $header = "POST /eng/query/fetch HTTP/1.0\r\n";
      $header .= 'Host: '. $host ."\r\n";
      $header .= "Content-Type: application/x-www-form-urlencoded\r\n";
      $header .= 'Content-Length: '. strlen( $req ) ."\r\n\r\n";
      
      // Connect to server
      $socket = fsockopen( 'ssl://'. $host, 443, $errno, $errstr, 10 );
      
      if(!$socket) {
        $error = true;
        print( 'errno = '. $errno .', errstr = '. $errstr );
      }
    }
 
    // Get data from server
    if(!$error) {
        // Send command to server
        fputs($socket, $header . $req);
        
        // Read the response from the server
        $res = '';
        $headerDone = false;
 
        while(!feof($socket)) {
            $line = fgets($socket, 1024);
            
            // Check if we are finished reading the header yet
            if(strcmp($line, "\r\n") == 0) {
              // read the header
              $headerDone = true;
            }
            // If header has been processed
            else if($headerDone) {
              // Read the main response
              $res .= $line;
            }
        }
 
        // Parse the returned data
        $lines = explode( "\n", $res );
    }
 
    // Interpret the response from server
    if(!$error) {
      $result = trim($lines[0]);
      
      // If the transaction was successful
      if(strcmp($result, 'SUCCESS') == 0 ) {
        // Process the reponse into an associative array of data
        for( $i = 1; $i < count($lines); $i++ ) {
          list( $key, $val ) = explode( "=", $lines[$i] );
          $data[urldecode( $key )] = stripslashes( urldecode( $val ) );
        }
      }
      // If the transaction was NOT successful
      else if(strcmp( $result, 'FAIL') == 0) {
        // Log for investigation
        $error = true;
      }
    }
 
    //// Process the payment
    if(!$error) {
      // Get the data from the new array as needed
      $nameFirst = $data['name_first'];
      $nameLast = $data['name_last'];
      $amountGross = $data['amount_gross'];

      // Once you have access to this data, you should perform a number of
      // checks to ensure the transaction is "correct" before processing it.
      // - Check the payment_status is Completed
      // - Check the pf_transaction_id has not already been processed
      // - Check the merchant_id is correct for your account

      // Process payment
      uc_cart_empty($cart_id);

      // redirect if cart setup to do that
      $page = variable_get('uc_cart_checkout_complete_page', '');
      if (!empty($page)) {
        drupal_goto($page);
      }
      
      // Otherwise just show order complete
      $output = uc_cart_complete_sale($order, variable_get('uc_new_customer_login', false));
      return $output;
    }
    
    // Close socket if successfully opened
    if($socket) fclose($socket);
  }
  
  // On a failure redirect back to the cart page.
  drupal_goto("cart");
}