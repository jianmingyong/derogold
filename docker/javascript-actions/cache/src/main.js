const core = require('@actions/core')
const cache = require('@actions/cache')

const cache_key = 'ccache'
const cache_dir = ['/root/.ccache']

/**
 * The main function for the action.
 * @returns {Promise<void>} Resolves when the action is complete.
 */
async function run() {
  try {
    if (cache.isFeatureAvailable()) {
      const actionType = core.getInput('action', { required: true })

      if (actionType === 'save') {
        const cacheId = await cache.saveCache(cache_dir, cache_key)
        core.setOutput('cache_id', cacheId)
      } else if (actionType === 'restore') {
        const cacheKey = cache.restoreCache(cache_dir, cache_key)
        core.setOutput('cache_key', cacheKey)
      }
    } else {
      core.error('GitHub Cache is not available.')
    }
  } catch (error) {
    // Fail the workflow run if an error occurs
    core.setFailed(error.message)
  }
}

module.exports = {
  run
}
