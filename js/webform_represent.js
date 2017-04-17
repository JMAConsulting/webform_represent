(function (window, Drupal, $) {
  var previousValues = {};

  Drupal.behaviors.webform_represent = {
    attach: function (context, settings) {
      webformRepresentSubmitEnabled();
      var postalCodeIds = settings.webform_represent.postalCodeIds;
      $.each(postalCodeIds, function(index, postalCodeId) {
        $('#' + postalCodeId).once('webform_represent').each(function (index) {
          var $postalCodeField = $(this);
          var domValue = getValueViaDomAttribute($postalCodeField);
          previousValues[postalCodeId] = domValue ? domValue : '';
          $postalCodeField.bind('input', function () {
            validator($postalCodeField, postalCodeId);
          });

          // Poll regularly to deal with autofill.
          window.setInterval(function () {
            validator($postalCodeField, postalCodeId);
          }, 1000);
        });
      });
    }
  };

  var validator = function($postalCodeField, postalCodeId) {
    var postalCode = $postalCodeField.val();
    if ((postalCode.length > 5) && (previousValues[postalCodeId] != postalCode)) {
      var postalCodeValidation = /^[a-zA-Z]\d[a-zA-Z]\s?\d[a-zA-Z]\d$/;
      if (postalCodeValidation.test(postalCode)) {
        // Use custom event to avoid interference from browser-generated events like "input" or "change".
        $postalCodeField.trigger('nmPostalCodeIsValid');
        previousValues[postalCodeId] = postalCode;
        return;
      }
      else {
        // TODO: Maybe we want to hide or clear the Representatives?
        // TODO: Maybe we should have a variable tracking valid/invalid
        //       postal code and if it changes, then do server reload
        // TODO: Invalid postal code should not allow for representative
        //       selection
      }
    }
  };

  // We only have jQuery 1.4.x, so we have to devise our own way to get to the
  // contents of the HTML "value" attribute.
  // @see http://api.jquery.com/attr/
  // @see http://api.jquery.com/prop/
  var getValueViaDomAttribute = function($inputField) {
    var element = $inputField.get(0);
    if ('object' == typeof element) {
      if ('attributes' in element) {
        var attributes = element.attributes;
        for (var i = attributes.length - 1; i >= 0; i--) {
          if ('value' == attributes[i].name) {
            return attributes[i].value;
          }
        }
      }
    }
    return null;
  }

  var webformRepresentSubmitEnabled = function () {
    var disabled = false;
    var postalCodeIds = Drupal.settings.webform_represent.postalCodeIds;
    $.each(postalCodeIds, function(index, postalCodeId){
      if (!$('#' + postalCodeId).val()) {
        disabled = true;
      }
    });
    var representNames = Drupal.settings.webform_represent.postalCodeIds;
    $.each(representNames, function(index, representName){
      if ($(':radio[name=' + representName + ']:checked').val()) {
        disabled = true;
      }
    });
    if (disabled) {
      $('input[type="submit"]').attr('disabled', true);
      $('input[type="submit"]').addClass('form-button-disabled');
    }
    else {
      $('input[type="submit"]').removeAttr('disabled');
      $('input[type="submit"]').removeClass('form-button-disabled');
    }
  };

}(window, Drupal, jQuery));
