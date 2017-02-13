<?php
/**
 * @file
 *
 * Custom drush command function.
 */

/**
 * Implements hook_drush_command().
 */
function custom_command_drush_command() {
  $items = array();

  $items['published-old-content'] = array(
    'description' => 'Published/Unpublished content older than 2 days',
    'aliases' => array('poc'),
    'arguments' => array(
//      'type' => 'Nodes of this type will be published/unpublished',
      'status' => 'Toggle status flag'
    ),
  );

  return $items;
}

function drush_custom_command_published_old_content($status) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node');
  $max = $query->count()->execute();
  $operations[] = array('custom_command_batch_operation', array($status, $max));

  $batch = array(
    'operations' => $operations,
    'finished' => 'custom_command_export_finished',
    'title' => t('Processing batch'),
    'init_message' => t('Batch is starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch has encountered an error.'),
  );
  // return the node objects.
//  $nodes = entity_load('node');
//  $nids = db_query('SELECT nid FROM {node}')->fetchCol();
//  $nodes = node_load_multiple($nids);

//  $count = 0;
  // UNIX timestamp for 60 seconds ago
//  $search_date = strtotime("- 60 seconds");

//  foreach ($nodes as $node) {
//    $last_update = $node->changed;
//
//    if (($search_date > $last_update) && $status == 0) {
//      $count++;
//      $node->status = 0;
//      node_save($node);
//    }
//    else if (($search_date > $last_update) && $status == 1) {
//      $count++;
//      $node->status = 1;
//      node_save($node);
//    }
//  }

//  if($count > 0) {
//    drush_print($count . " nodes have been successfully publish status changed.");
//  }
//  else {
//    drush_print("No nodes were published.");
//  }

  batch_set($batch);

  drush_backend_batch_process();
}

/**
 * @param $status
 * @param $max
 * @param $context
 */
function custom_command_batch_operation($status, $max, &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = $max;
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->propertyCondition('created', time()-60, '<')
    ->propertyCondition('status', $status ? 0 : 1)
    ->propertyCondition('nid', $context['sandbox']['current_node'], '>')
    ->range(NULL, 2);
  $record = $query->execute();
  $time = microtime(TRUE) + 0.03;
  $current_time = microtime(TRUE);

  if ($current_time < $time) {
    if (count($record['node']) == 0) {
      return;
    }
    $list_ids = array();
    foreach ($record['node'] as $nid) {
      $list_ids[] = $nid->nid;
    }
    $nodes = node_load_multiple($list_ids);

    foreach ($nodes as $node) {
      if ($status == NODE_NOT_PUBLISHED) {
        $node->status = 0;
        node_save($node);
      }
      else if ($status == NODE_PUBLISHED) {
        $node->status = 1;
        node_save($node);
      }

      $context['results']['titles'][] = "Result: title of node";
      $context['sandbox']['progress']++;
      $context['sandbox']['current_node'] = $node->nid;
      $context['message'] = "Status of node with type ".check_plain($node->type). " was changed";
    }
    $current_time = microtime(TRUE);
  }
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
  }
}

function custom_command_export_finished($success, $results, $operations) {
  if ($success) {
    drush_print('Updated status on node that older than 60 seconds');
  }
  else {
    drush_print('Finished with errors.');
  }
}
